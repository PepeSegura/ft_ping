FROM debian:latest

# Install dependencies including sudo
RUN apt-get update && \
    apt-get install -y zsh build-essential curl git btop man wget gcc make sudo && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Create non-root user and add to sudo group
RUN useradd -m -s /bin/zsh appuser && \
    usermod -aG sudo appuser && \
    echo "appuser ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Download, compile, and install inetutils 2.0 (for ping)
RUN wget https://ftp.gnu.org/gnu/inetutils/inetutils-2.0.tar.gz && \
    tar -xvf inetutils-2.0.tar.gz && \
    cd inetutils-2.0 && \
    ./configure && \
    make && \
    make install && \
    cd .. && \
    rm -rf inetutils-2.0 inetutils-2.0.tar.gz

# Install Oh My Zsh as non-root user
USER appuser
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended

# Set default user and entrypoint
USER appuser
CMD ["zsh"]