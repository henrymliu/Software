#!/bin/bash

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
# UBC Thunderbots Arch Linux Software Setup
#
# This script must be run with sudo! root permissions are required to install
# packages and copy files to the /etc/udev/rules.d directory. The reason that the script
# must be run with sudo rather than the individual commands using sudo, is that
# when running CI within Docker, the sudo command does not exist since
# everything is automatically run as root.
#
# This script will install all the required libraries and dependencies to build
# and run the Thunderbots codebase. This includes being able to run the ai and
# unit tests
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#


# Save the parent dir of this so we can always run commands relative to the
# location of this script, no matter where it is called from. This
# helps prevent bugs and odd behaviour if this script is run through a symlink
# or from a different directory.
CURR_DIR=$(dirname -- "$(readlink -f -- "$BASH_SOURCE")")
cd $CURR_DIR
# The root directory of the reopsitory and git tree
GIT_ROOT="$CURR_DIR/.."


echo "================================================================"
echo "Setting up your shell config files"
echo "================================================================"
# Shell config files that various shells source when they run.
# This is where we want to add aliases, source ROS environment
# variables, etc.
SHELL_CONFIG_FILES=(
    "$HOME/.bashrc"\
    "$HOME/.zshrc"
)

# All lines listed here will be added to the shell config files
# listed above, if they are not present already
# These automatically perform functions outlined in the ROS setup tutorial
# http://wiki.ros.org/melodic/Installation/Ubuntu
declare -a new_shell_config_lines=(
    # Source the ROS Environment Variables Automatically
    "source /opt/ros/melodic/setup.sh"\
    # Source the setup script for our workspace. This normally needs to
    # be done manually each session before you can work on the workspace,
    # so we put it here for convenience.
    "source $GIT_ROOT/devel/setup.sh"\
    # Aliases to make development easier
    # You need to source the setup.sh script before launching CLion so it can
    # find catkin packages
    "alias clion=\"source /opt/ros/melodic/setup.sh \
        && source $GIT_ROOT/devel/setup.sh \
        && clion & disown \
        && exit\""\
    "alias rviz=\"rviz & disown && exit\""\
    "alias rqt=\"rqt & disown && exit\""
    "alias catkin_make=\"catkin_make --cmake-args -DPYTHON_VERSION=3\""
)

# Add all of our new shell config options to all the shell
# config files, but only if they don't already have them
for file_name in "${SHELL_CONFIG_FILES[@]}";
do
    if [ -f "$file_name" ]
    then
        echo "Setting up $file_name"
        for line in "${new_shell_config_lines[@]}";
        do
            if ! grep -Fq "$line" $file_name
            then
                echo "$line" >> $file_name
            fi
        done
    fi
done

# Install ROS
echo "================================================================" 
echo "Installing ROS Melodic, MAKE SURE TO SELECT PYTHON 3"
echo "================================================================"

# See http://wiki.ros.org/melodic/Installation/Ubuntu for instructions
yay -S ros-melodic-ros-base

if [ $? -ne 0 ]; then
    echo "##############################################################"
    echo "Error: Installing ROS failed"
    echo "##############################################################"
    exit 1
fi
yay -S python-rosinstall python-rosinstall-generator python-wstool


echo "================================================================"
echo "Installing other ROS dependencies specified by our packages"
echo "================================================================"

# Update Rosdeps
# rosdep init only needs to be called once after installation.
# Check if the sources.list already exists to determing if rosdep has
# been installed already
if [ ! -f "/etc/ros/rosdep/sources.list.d/20-default.list" ]
then
    sudo rosdep init
fi

rosdep update
yay -S --noconfirm ros-melodic-rqt-reconfigure
echo "================================================================"
echo "Installing Misc. Utilities"
echo "================================================================"

# Nodejs and yarn for corner)kick
yay -S nodejs-lts-carbon yarn

host_software_packages=(
    python-rosinstall
    protobuf
    libusb
    eigen # A math / numerical library used for things like linear regression
)
yay -S --noconfirm ${host_software_packages[@]}

if [ $? -ne 0 ]; then
    echo "##############################################################"
    echo "Error: Installing utilities failed"
    echo "##############################################################"
    exit 1
fi

# Clone, build, and install g3log. Adapted from instructions at:
# https://github.com/KjellKod/g3log
g3log_path="/tmp/g3log"
if [ -d $g3log_path ]; then
    echo "Removing old g3log..."
    sudo rm -r $g3log_path
fi

git clone https://github.com/KjellKod/g3log.git $g3log_path
cd $g3log_path
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
cd $CURR_DIR

# Clone, build, and install munkres-cpp (Our Hungarian library algorithm)
hungarian_path="/tmp/hungarian-cpp"
if [ -d $hungarian_path ]; then
    echo "Removing old hungarian-cpp library..."
    sudo rm -r $hungarian_path
fi

git clone https://github.com/saebyn/munkres-cpp.git $hungarian_path
cd $hungarian_path
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
cd $CURR_DIR

# yaml for cfg generation (Dynamic Parameters)
yay -S --noconfirm python-yaml

# Done
echo "================================================================"
echo "Done Software Setup"
echo "================================================================"

# Additional changes to rqt_reqconfigure
# timestamps in param_updater.py
# log statement in dynreconf_clieng_widget.py
# index in paramedit_widget.py
