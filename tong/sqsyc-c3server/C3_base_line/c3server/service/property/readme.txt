
docker run -it --rm -v ${PWD}:/app_work/ -w /app_work ag35xxt:r01a04 bash -c "cd /app_work/app && mkdir -p build && mkdir -p /app_work/app/out/apps && rm build/* -rf  && cd build && . /opt/ql_crosstools/crosstool-env-init && cmake .. -DCMAKE_INSTALL_PREFIX=/app_work/app/out -DMODEMTYPE=ag35cet && make property_server -j4 && mv property_server/property_server /app_work/app/out/apps/"
