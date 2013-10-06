cd /home/vagrant

# run vagrant post install
#if [ -f /home/vagrant/postinstall.sh ];
#then
#  ./postinstall.sh
#  rm postinstall.sh
#fi

# TODO: there's probably a better way to handle this, but this gets around warnings
git config --global user.email "no@no.com"
git config --global user.name "EspruinoBuildEnvironment"

# install stlink
if [ ! -f /home/vagrant/stlink ];
then
  # get stlink source
  git clone https://github.com/texane/stlink.git
  cd stlink

  # build stlink
  ./autogen.sh
  ./configure
  make
  cd ../

  # fix permissions
  chown vagrant:vagrant -R stlink
fi

if [ ! -f /home/vagrant/arm-eabi-toolchain ];
then
  # get ARM EABI Toolchain Builder
  git clone https://github.com/jsnyder/arm-eabi-toolchain.git

  chown vagrant:vagrant -R arm-eabi-toolchain

  # TODO: find out why blueprint misses this, or just add to chef config manually
  apt-get install texinfo

  # make!
  cd arm-eabi-toolchain
  #PROCS=2 make install-cross
  make install-cross

  # add tools path
  cd ../
  echo "PATH=/home/vagrant/arm-cs-tools/bin:$PATH" > /home/vagrant/.profile
fi
