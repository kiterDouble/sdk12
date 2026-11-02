#include "tsp_http.h"
#include "tsp_log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

/* ======================== 简易JSON解析 ======================== */

static char *json_find_key(const char *json, const char *key)
{
    if (!json || !key) return NULL;

    char search[256];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *p = strstr(json, search);
    if (!p) return NULL;

    p += strlen(search);
    while (*p && (*p == ' ' || *p == ':' || *p == '\t')) p++;
    if (!*p) return NULL;

    if (*p == '"') {
        p++;
        const char *end = strchr(p, '"');
        if (!end) return NULL;
        size_t len = (size_t)(end - p);
        char *val = (char *)malloc(len + 1);
        if (!val) return NULL;
        memcpy(val, p, len);
        val[len] = '\0';
        return val;
    }

    if (*p == '-' || (*p >= '0' && *p <= '9')) {
        const char *end = p;
        while (*end && *end != ',' && *end != '}' && *end != ']') end++;
        size_t len = (size_t)(end - p);
        char *val = (char *)malloc(len + 1);
        if (!val) return NULL;
        memcpy(val, p, len);
        val[len] = '\0';
        return val;
    }

    if (*p == '{' || *p == '[') {
        char open = *p, close = (*p == '{') ? '}' : ']';
        int depth = 1;
        const char *end = p + 1;
        while (*end && depth > 0) {
            if (*end == open) depth++;
            else if (*end == close) depth--;
            else if (*end == '"') { end++; while (*end && *end != '"') { if (*end == '\\') end++; end++; } }
            end++;
        }
        size_t len = (size_t)(end - p);
        char *val = (char *)malloc(len + 1);
        if (!val) return NULL;
        memcpy(val, p, len);
        val[len] = '\0';
        return val;
    }

    return NULL;
}

static int json_find_int(const char *json, const char *key, int def_val)
{
    char *val = json_find_key(json, key);
    if (!val) return def_val;
    int result = atoi(val);
    free(val);
    return result;
}

/* ======================== URL编码 ======================== */

static int url_encode_char(char *out, size_t out_size, char c)
{
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
        return snprintf(out, out_size, "%c", c);
    }
    return snprintf(out, out_size, "%%%02X", (unsigned char)c);
}

static size_t url_encode(const char *src, char *dst, size_t dst_size)
{
    if (!src || !dst || dst_size == 0) return 0;

    size_t di = 0;
    for (const char *s = src; *s && di < dst_size - 1; s++) {
        int n = url_encode_char(dst + di, dst_size - di, *s);
        if (n < 0) break;
        di += (size_t)n;
    }
    dst[di] = '\0';
    return di;
}

/* ======================== HTTP GET (POSIX socket) ======================== */

static char *http_get_request(const char *url)
{
    char host[256] = {0}, path[2048] = {0};
    int port = 80;

    if (strncmp(url, "http://", 7) == 0) {
        const char *hp = url + 7;
        const char *slash = strchr(hp, '/');
        if (slash) {
            size_t hlen = (size_t)(slash - hp);
            if (hlen >= sizeof(host)) hlen = sizeof(host) - 1;
            memcpy(host, hp, hlen);
            strncpy(path, slash, sizeof(path) - 1);
        } else {
            strncpy(host, hp, sizeof(host) - 1);
            strncpy(path, "/", sizeof(path) - 1);
        }

        char *colon = strchr(host, ':');
        if (colon) {
            *colon = '\0';
            port = atoi(colon + 1);
        }
    } else {
        TLY_LOG_E("Unsupported URL scheme: %s", url);
        return NULL;
    }

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    int rc = getaddrinfo(host, port_str, &hints, &res);
    if (rc != 0) {
        TLY_LOG_E("Cannot resolve %s: %s", host, gai_strerror(rc));
        return NULL;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        freeaddrinfo(res);
        TLY_LOG_E("Socket create failed: %s", strerror(errno));
        return NULL;
    }

    struct timeval tv = {10, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    rc = connect(fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (rc != 0) {
        close(fd);
        TLY_LOG_E("Connect to %s:%d failed: %s", host, port, strerror(errno));
        return NULL;
    }

    char req[4096];
    int req_len = snprintf(req, sizeof(req),
        "GET %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "Accept: application/json\r\n"
        "\r\n",
        path, host);

    ssize_t sent = send(fd, req, (size_t)req_len, 0);
    if (sent != req_len) {
        close(fd);
        TLY_LOG_E("Send failed: %s", strerror(errno));
        return NULL;
    }

    size_t body_cap = 8192;
    char *body = (char *)malloc(body_cap);
    if (!body) { close(fd); return NULL; }
    size_t body_len = 0;

    char recv_buf[4096];
    ssize_t n;
    int header_end = 0;

    while ((n = recv(fd, recv_buf, sizeof(recv_buf), 0)) > 0) {
        if (!header_end) {
            char *hdr_end = strstr(recv_buf, "\r\n\r\n");
            if (hdr_end) {
                header_end = 1;
                size_t hdr_total = (size_t)(hdr_end - recv_buf) + 4;
                size_t body_chunk = (size_t)n - hdr_total;
                if (body_chunk > 0) {
                    if (body_len + body_chunk + 1 > body_cap) {
                        body_cap = (body_len + body_chunk + 1) * 2;
                        char *new_body = (char *)realloc(body, body_cap);
                        if (!new_body) { free(body); close(fd); return NULL; }
                        body = new_body;
                    }
                    memcpy(body + body_len, recv_buf + hdr_total, body_chunk);
                    body_len += body_chunk;
                }
            }
        } else {
            if (body_len + (size_t)n + 1 > body_cap) {
                body_cap = (body_len + (size_t)n + 1) * 2;
                char *new_body = (char *)realloc(body, body_cap);
                if (!new_body) { free(body); close(fd); return NULL; }
                body = new_body;
            }
            memcpy(body + body_len, recv_buf, (size_t)n);
            body_len += (size_t)n;
        }
    }

    close(fd);

    if (body_len == 0) { free(body); return NULL; }
    body[body_len] = '\0';
    return body;
}

/* ======================== 统一GET+JSON解析 ======================== */

static int http_get_json(const char *url, tly_http_resp_t *resp_out)
{
    if (!url || !resp_out) return TLY_ERR_PARAM;
    memset(resp_out, 0, sizeof(*resp_out));

    char *body = http_get_request(url);
    if (!body) {
        TLY_LOG_E("HTTP GET failed: %s", url);
        return TLY_ERR_HTTP;
    }

    TLY_LOG_D("HTTP GET response: %s", body);

    resp_out->code = json_find_int(body, "code", -1);

    char *msg = json_find_key(body, "msg");
    if (msg) resp_out->msg = msg;

    char *data = json_find_key(body, "data");
    if (data) resp_out->data_json = data;

    free(body);
    return 0;
}

/* ======================== 全局初始化/清理 ======================== */

int tly_http_global_init(void)
{
    return 0;
}

void tly_http_global_cleanup(void)
{
}

/* ======================== URL拼接 ======================== */

static int build_url(char *buf, size_t buf_size, const char *base_url,
                     const char *path, ...)
{
    va_list ap;
    va_start(ap, path);

    int n = snprintf(buf, buf_size, "%s%s?", base_url, path);
    if (n < 0 || (size_t)n >= buf_size) { va_end(ap); return TLY_ERR_PARAM; }

    size_t pos = (size_t)n;
    int first = 1;

    while (1) {
        const char *key = va_arg(ap, const char *);
        if (!key) break;
        const char *val = va_arg(ap, const char *);
        if (!val) break;

        char encoded[1024];
        url_encode(val, encoded, sizeof(encoded));

        n = snprintf(buf + pos, buf_size - pos,
                     "%s%s=%s", first ? "" : "&", key, encoded);
        if (n < 0 || (size_t)(pos + n) >= buf_size) { va_end(ap); return TLY_ERR_PARAM; }
        pos += (size_t)n;
        first = 0;
    }

    va_end(ap);
    return 0;
}

static const char *int_to_str(int val, char *buf, size_t buf_size)
{
    snprintf(buf, buf_size, "%d", val);
    return buf;
}

/* ======================== API实现 ======================== */

void tly_http_resp_free(tly_http_resp_t *resp)
{
    if (resp) {
        free(resp->msg);
        free(resp->data_json);
        resp->msg = NULL;
        resp->data_json = NULL;
    }
}

int tly_http_lock(tly_http_resp_t *resp, const char *base_url,
                  const tly_lock_params_t *params)
{
    if (!resp || !base_url || !params) return TLY_ERR_PARAM;

    char lt[16], es[16], tq[16], et[16];
    char url[1024];

    int rc = build_url(url, sizeof(url), base_url, "/control/lock",
                       "tboxVin", params->tbox_vin,
                       "lockType", int_to_str(params->lock_type, lt, sizeof(lt)),
                       "engineSpeed", int_to_str(params->engine_speed, es, sizeof(es)),
                       "torque", int_to_str(params->torque, tq, sizeof(tq)),
                       "engineType", int_to_str(params->engine_type, et, sizeof(et)),
                       "operator", params->operator_name ? params->operator_name : "",
                       "vin", params->vin ? params->vin : "",
                       NULL);
    if (rc != 0) return rc;

    TLY_LOG_I("Lock request: VIN=%s, type=%d", params->tbox_vin, params->lock_type);
    return http_get_json(url, resp);
}

int tly_http_unbind(tly_http_resp_t *resp, const char *base_url,
                    const char *new_tbox_vin, const char *old_tbox_vin,
                    int engine_type, const char *operator_name)
{
    if (!resp || !base_url) return TLY_ERR_PARAM;

    char et[16];
    char url[1024];

    int rc = build_url(url, sizeof(url), base_url, "/control/unbind",
                       "newTboxVin", new_tbox_vin,
                       "oldTboxVin", old_tbox_vin,
                       "engineType", int_to_str(engine_type, et, sizeof(et)),
                       "operator", operator_name ? operator_name : "",
                       NULL);
    if (rc != 0) return rc;

    TLY_LOG_I("Unbind request: new=%s, old=%s", new_tbox_vin, old_tbox_vin);
    return http_get_json(url, resp);
}

int tly_http_ota_upgrade(tly_http_resp_t *resp, const char *base_url,
                         const tly_ota_params_t *params)
{
    if (!resp || !base_url || !params) return TLY_ERR_PARAM;

    char vt[16], ut[16], dt[16];
    char url[2048];

    int rc = build_url(url, sizeof(url), base_url, "/ota/upgrade",
                       "tboxVin", params->tbox_vin,
                       "ftpUrl", params->ftp_url,
                       "username", params->username,
                       "password", params->password,
                       "vehicleType", int_to_str(params->vehicle_type, vt, sizeof(vt)),
                       "upgradeType", int_to_str(params->upgrade_type, ut, sizeof(ut)),
                       "deviceType", int_to_str(params->device_type, dt, sizeof(dt)),
                       NULL);
    if (rc != 0) return rc;

    TLY_LOG_I("OTA upgrade: VIN=%s, vehicle=%d, device=%d",
              params->tbox_vin, params->vehicle_type, params->device_type);
    return http_get_json(url, resp);
}

int tly_http_get_sd_data(tly_http_resp_t *resp, const char *base_url,
                         const char *tbox_vin,
                         const char *ftp_url, const char *username,
                         const char *password, int vehicle_type,
                         const char *begin_time, int time_count)
{
    if (!resp || !base_url) return TLY_ERR_PARAM;

    char vt[16], tc[16];
    char url[2048];

    int rc = build_url(url, sizeof(url), base_url, "/sd/getSdData",
                       "tboxVin", tbox_vin,
                       "ftpUrl", ftp_url,
                       "username", username,
                       "password", password,
                       "vehicleType", int_to_str(vehicle_type, vt, sizeof(vt)),
                       "beginTime", begin_time,
                       "timeCount", int_to_str(time_count, tc, sizeof(tc)),
                       NULL);
    if (rc != 0) return rc;

    TLY_LOG_I("SD data request: VIN=%s, begin=%s", tbox_vin, begin_time);
    return http_get_json(url, resp);
}

int tly_http_seed_key(tly_http_resp_t *resp, const char *base_url,
                      const char *tbox_vin, const char *seed)
{
    if (!resp || !base_url) return TLY_ERR_PARAM;

    char url[1024];

    int rc = build_url(url, sizeof(url), base_url, "/seed/key",
                       "tboxVin", tbox_vin,
                       "seed", seed,
                       NULL);
    if (rc != 0) return rc;

    TLY_LOG_I("Seed key request: VIN=%s", tbox_vin);
    return http_get_json(url, resp);
}

int tly_http_time_sync(tly_http_resp_t *resp, const char *base_url,
                       const char *tbox_vin, const char *cmd_type,
                       const char *vehicle_type, int time_second)
{
    if (!resp || !base_url) return TLY_ERR_PARAM;

    char ts[16];
    char url[1024];

    int rc = build_url(url, sizeof(url), base_url, "/ts/push",
                       "tboxVin", tbox_vin,
                       "cmdType", cmd_type,
                       "vehicleType", vehicle_type,
                       "timeSecond", int_to_str(time_second, ts, sizeof(ts)),
                       NULL);
    if (rc != 0) return rc;

    TLY_LOG_I("Time sync request: VIN=%s, cmd=%s, period=%ds",
              tbox_vin, cmd_type, time_second);
    return http_get_json(url, resp);
}

int tly_http_battery_heat(tly_http_resp_t *resp, const char *base_url,
                          const char *tbox_vin, const char *cmd_type)
{
    if (!resp || !base_url) return TLY_ERR_PARAM;

    char url[1024];

    int rc = build_url(url, sizeof(url), base_url, "/bhs/heat",
                       "tboxVin", tbox_vin,
                       "cmdType", cmd_type,
                       NULL);
    if (rc != 0) return rc;

    TLY_LOG_I("Battery heat request: VIN=%s, cmd=%s", tbox_vin, cmd_type);
    return http_get_json(url, resp);
}
