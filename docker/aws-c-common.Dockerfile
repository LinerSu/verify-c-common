FROM diffblue/cbmc:5.56.0

## install required pacakges
USER root

RUN apt-get update && apt -y install software-properties-common
RUN add-apt-repository 'ppa:ubuntu-toolchain-r/test'
RUN apt-get update && \
    apt-get install -y python3.8 python3-pip gcc-10 g++-10 ninja-build gnuplot \
    flex bison make wget git libwww-perl patch ccache libc6-dev-i386 libc6-dev jq cmake && \
    apt-get clean

RUN rm /usr/bin/python3 && ln /usr/bin/python3.8 /usr/bin/python3
RUN pip3 install jinja2 voluptuous

WORKDIR /home/
RUN rm -rf aws-c-common && git clone https://github.com/awslabs/aws-c-common.git
RUN cd aws-c-common/verification/cbmc && git checkout main && \
    git submodule init && git submodule update

WORKDIR /home/aws-c-common/verification/cbmc/proofs/scripts/
COPY scripts/run_aws_res.py .

WORKDIR /home/aws-c-common/verification/cbmc/proofs/
