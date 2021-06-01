ARG CMONGO_VERSION=1.0b-12
ARG CERVER_VERSION=2.0b-36

ARG BUILD_DEPS='libssl-dev'
ARG RUNTIME_DEPS='libssl-dev'

FROM ermiry/mongoc:builder as builder

WORKDIR /opt

ARG BUILD_DEPS
RUN apt-get update && apt-get install -y ${BUILD_DEPS}

# build cmongo with production flags
ARG CMONGO_VERSION
RUN mkdir /opt/cmongo && cd /opt/cmongo \
    && wget -q --no-check-certificate https://github.com/ermiry-com/cmongo/archive/${CMONGO_VERSION}.zip \
    && unzip ${CMONGO_VERSION}.zip \
    && cd cmongo-${CMONGO_VERSION} \
    && make TYPE=production -j4 && make TYPE=production install

# build cerver with production flags
ARG CERVER_VERSION
RUN mkdir /opt/cerver && cd /opt/cerver \
    && wget -q --no-check-certificate https://github.com/ermiry/cerver/archive/${CERVER_VERSION}.zip \
    && unzip ${CERVER_VERSION}.zip \
    && cd cerver-${CERVER_VERSION} \
    && make TYPE=production -j4 && make TYPE=production install

RUN ldconfig

# todo
WORKDIR /opt/todo
COPY . .
RUN make TYPE=production -j4

############
FROM ermiry/mongoc:latest

ARG RUNTIME_DEPS
RUN apt-get update && apt-get install -y ${RUNTIME_DEPS}

# cmongo
ARG CMONGO_VERSION
COPY --from=builder /opt/cmongo/cmongo-${CMONGO_VERSION}/bin/libcmongo.so /usr/local/lib/
COPY --from=builder /opt/cmongo/cmongo-${CMONGO_VERSION}/include/cmongo /usr/local/include/cmongo

# cerver
ARG CERVER_VERSION
COPY --from=builder /opt/cerver/cerver-${CERVER_VERSION}/bin/libcerver.so /usr/local/lib/
COPY --from=builder /opt/cerver/cerver-${CERVER_VERSION}/include/cerver /usr/local/include/cerver

RUN ldconfig

# todo
WORKDIR /home/todo
COPY ./start.sh .
COPY --from=builder /opt/todo/bin ./bin

CMD ["/bin/bash", "start.sh"]