#!/bin/sh

install_packages() {
    case $1 in
        "ubuntu" | "debian")
            apt-get install $OPT tmux pv reptyr sysstat
            exit
            ;;
        "fedora")
            dnf install $OPT tmux pv reptyr sysstat
            exit
            ;;
        "rhel" | "centos")
            rpm -ivh https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
            yum install $OPT tmux pv reptyr sysstat
            exit
            ;;
        "arch" | "manjaro")
            pacman $OPT -S tmux pv reptyr sysstat
            exit
            ;;
        "alpine")
            apk add $OPT tmux pv reptyr sysstat
            exit
            ;;
    esac
}

if [ "x$(id -u)" != x0 ]; then
    echo "Please run it again with 'sudo'."
    exit
fi

OPT="${@}"

if [ ! -f /etc/os-release ]; then
    echo "not supported, install manually."
    exit
fi

distro=$(grep "^ID=" /etc/os-release | cut -d\= -f2 | sed -e 's/"//g')
id_like=$(grep "^ID_LIKE=" /etc/os-release | cut -d\= -f2 | sed -e 's/"//g')

install_packages "$distro"

for distro_like in $id_like; do
    install_packages "$distro_like"
    echo 0 > /proc/sys/kernel/yama/ptrace_scope
done

echo "\"$distro\" not supported, install manually."
echo

# sh -c 'while :; do printf "1 "; sleep 1; done'

// 디렉토리 내 파일 개수 출력
# sh -c 'while :; do ls | wc -l; sleep 2; done' 

// CPU 사용량 출력
# sh -c 'while :; do mpstat 1 1; done'
