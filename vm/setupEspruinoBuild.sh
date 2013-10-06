cd /home/vagrant

# run vagrant post install
#echo "postinstall"
#if [ -f postinstall.sh ];
#then
#  ./postinstall.sh
#  rm postinstall.sh
#fi

echo "git config"
# TODO: there's probably a better way to handle this
git config --global user.email "no@no.com"
git config --global user.name "EspruinoBuildEnvironment"

# install stlink
git clone https://github.com/texane/stlink.git
cd stlink
sudo ./autogen.sh
./configure
make
cd ../
chown vagrant:vagrant -R stlink

# get ARM EABI Toolchain Builder
#git clone https://github.com/jsnyder/arm-eabi-toolchain.git

# make!
#cd arm-eabi-toolchain
#PROCS=2 make install-cross
