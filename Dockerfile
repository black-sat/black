ARG VERSION=rawhide
FROM fedora:$VERSION
RUN echo 'fastestmirror=1' >> /etc/dnf/dnf.conf
RUN yum update -y
RUN yum install -y zsh wget zsh-syntax-highlighting git-lfs
RUN yum install -y make cmake gcc gcc-c++ \
                   fmt-devel catch-devel git \
                   z3-devel cvc5-devel \
                   python3


RUN git clone https://github.com/arximboldi/immer.git
WORKDIR /immer/build
RUN cmake ..
RUN make install

RUN cd ~ && wget https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh && sh install.sh
RUN sed -i s/robbyrussell/avit/g ~/.zshrc
RUN echo "source /usr/share/zsh-syntax-highlighting/zsh-syntax-highlighting.zsh" >> ~/.zshrc

ENTRYPOINT ["/usr/bin/env", "--"]
