FROM debian:latest

# Install basic dependencies
RUN apt-get update && \
    apt-get install -y zsh build-essential curl git btop man wget gcc make && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Download, compile, and install inetutils 2.0 (for ping)
RUN wget https://ftp.gnu.org/gnu/inetutils/inetutils-2.0.tar.gz && \
    tar -xvf inetutils-2.0.tar.gz && \
    cd inetutils-2.0 && \
    ./configure && \
    make && \
    make install && \
    cd .. && \
    rm -rf inetutils-2.0 inetutils-2.0.tar.gz

# Install Oh My Zsh
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended

# Set Zsh as default shell
RUN chsh -s $(which zsh)

CMD ["zsh"]
