#!/bin/sh

echo "Installing required packages"

# Remove files if any
sudo rm -rf mongo-c-driver-1.17.3
sudo rm -f mongo-c-driver-1.17.3.tar.gz

# Update the package index files
sudo apt-get update
# Run Upgrade
sudo apt-get -y upgrade

# Install required packages
sudo apt-get install -y build-essential
sudo apt-get install -y cmake
sudo apt-get install -y libssl-dev
sudo apt-get install -y zlib1g-dev
sudo apt-get install -y curl
# Required for Guid
sudo apt-get install -y uuid-dev 

echo "Installing MongoDB"

# Import public key
wget -qO - https://www.mongodb.org/static/pgp/server-4.4.asc | sudo apt-key add -
# Create a list file for MongoDB
echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu focal/mongodb-org/4.4 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-4.4.list

# Update the package index files
sudo apt-get update

# Install the latest version of MongoDB
sudo apt-get install -y mongodb-org

# Pin the packages at the currently installed versions
echo "mongodb-org hold" | sudo dpkg --set-selections
echo "mongodb-org-server hold" | sudo dpkg --set-selections
echo "mongodb-org-shell hold" | sudo dpkg --set-selections
echo "mongodb-org-mongos hold" | sudo dpkg --set-selections
echo "mongodb-org-tools hold" | sudo dpkg --set-selections

sudo systemctl daemon-reload
# Start MongoDB
sudo systemctl start mongod

echo "Installing MongoDB C Driver"

wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.3/mongo-c-driver-1.17.3.tar.gz
tar xzf mongo-c-driver-1.17.3.tar.gz
cd mongo-c-driver-1.17.3
mkdir cmake-build
cd cmake-build
cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ..
cmake --build .
sudo cmake --build . --target install

echo "Installing MongoDB C++ Driver"

echo "Downloading MongoDB C++ Driver"

# Download the latest version of mongocxx driver
curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.6.2/mongo-cxx-driver-r3.6.2.tar.gz
tar -xzf mongo-cxx-driver-r3.6.2.tar.gz
cd mongo-cxx-driver-r3.6.2/build

echo "Configuring MongoDB C++ Driver"

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local

echo "Building and Installing MongoDB C++ Driver"

sudo cmake --build . --target EP_mnmlstc_core
cmake --build .
sudo cmake --build . --target install

# Add usr/local/lib to the library path
# Used for shared libraries
sudo sh -c "printf '/usr/local/lib' > /etc/ld.so.conf.d/shared-library-path.conf"
sudo ldconfig

# Remove files
sudo rm -rf mongo-c-driver-1.17.3
sudo rm -f mongo-c-driver-1.17.3.tar.gz

# Stop mongod
sudo systemctl stop mongod

# Deploy a replica set 
sudo mkdir -p /srv/mongodb/db0
sudo chown -R mongodb:mongodb /srv/mongodb/db0 
sudo mongod --port 27017 --dbpath /srv/mongodb/db0 --replSet rs0 --bind_ip localhost --fork --logpath /var/log/mongod.log
mongo --eval "rs.initiate()"

curl -sL https://aka.ms/InstallAzureCLIDeb | sudo bash

echo "Installation complete!"
