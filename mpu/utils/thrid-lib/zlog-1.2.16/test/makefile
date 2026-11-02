exe = 		\
	test_tmp	\
	test_longlog	\
	test_buf	\
	test_bitmap	\
	test_conf	\
	test_conf2  \
	test_hashtable	\
	test_hello	\
	test_hex	\
	test_init	\
	test_level	\
	test_leak	\
	test_mdc	\
	test_multithread	\
	test_record	\
	test_pipe	\
	test_press_zlog		\
	test_press_zlog2	\
	test_press_write	\
	test_press_write2	\
	test_press_syslog	\
	test_syslog	\
	test_default	\
	test_profile	\
	test_category   \
	test_prompt	\
	test_enabled \
	test

all     :       $(exe)

$(exe)  :       %:%.o
	${CC} -O2 -g -o $@ $^ -L/opt/ag35/ql-ol-sdk/ql-ol-crosstool/sysroots/armv7ahf-neon-oe-linux-gnueabi/usr/lib -lzlog -lpthread -Wl,-rpath ../src

.c.o	:
	${CC} -O2 -g -Wall -D_GNU_SOURCE -o $@ -c $< -I. -I../src

clean	:
	rm -f press.log* *.o $(exe)

.PHONY : clean all
