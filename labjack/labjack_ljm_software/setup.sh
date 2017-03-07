#! /bin/bash
#
# LJM software-stack installer for Linux to be run by Makeself.
#
# Installs LJM, Exodriver (liblabjackusb), Kipling.
# Sets up ldconfig.
# Sets up and restarts the device rules.
# Displays instructions.

set -e
set -u

# LJM version in a format like 1.14.2
LJM_VERSION=$1

# Exodriver (liblabjackusb) version in a format like 2.6.0
LABJACK_USB_VERSION=$2

LIB_DESTINATION=/usr/local/lib
CONSTANTS_DESTINATION=/usr/local/share
HEADER_DESTINATION=/usr/local/include
PROGRAM_DESTINATION=/opt
BIN_DESTINATION=/usr/local/bin

LJM_MAJOR_V=${LJM_VERSION%%.*}
LJM_LIB=libLabJackM.so
LJM_REALNAME=$LJM_LIB.$LJM_VERSION
LJM_SONAME=$LJM_LIB.$LJM_MAJOR_V
LJM_LINKERNAME=$LJM_LIB
LJM_HEADER="LabJackM.h"

LIB_LABJACK_USB_VERSIONED=liblabjackusb.so.$LABJACK_USB_VERSION
LIB_LABJACK_USB=liblabjackusb.so

# The name of the directory that Kipling is packaged in, as well as the name of
# the directory where it is installed
KIPLING_INSTALL_DIR=labjack_kipling

# The name of the Kipling command-line launcher
KIPLING_BIN_SHORTCUT=labjack_kipling

# Exodriver
RULES=10-labjack.rules
OLD_RULES=50-labjack.rules
RULES_DEST_PRIMARY=/lib/udev/rules.d
RULES_DEST_ALTERNATE=/etc/udev/rules.d
GROUP=adm

LDCONFIG_FILE=/etc/ld.so.conf

SUPPORT_EMAIL=support@labjack.com

TRUE="true"
FALSE="false"

# Assume these are unneeded until otherwise
NEED_RECONNECT=$FALSE
NEED_RESTART=$FALSE
NEED_RELOG=$FALSE
NO_RULES=$FALSE
NO_RULES_ERR=2
SKIP_USER_PERMISSIONS=$FALSE
USER_REALLY_IS_ROOT=$FALSE


# Function declarations

success ()
{
	echo

	cd ..
	go rm -rf "./labjack_ljm_software"

	# Change permissions
	if [ $SKIP_USER_PERMISSIONS == $FALSE ]; then
		chown --recursive $user:$GROUP labjack_ljm_examples
		if [ $? -ne 0 ]; then
			echo "chown failure. You may want to manually chown labjack_ljm_examples"
			echo "dir, although it is not required."
			echo
		fi
	fi

	e=0
	echo "Install finished. Please check out the README for usage help."
	if [ $NEED_RECONNECT == $TRUE ]; then
		echo
		echo "If you have any LabJack devices connected, please disconnect and"
		echo "reconnect them now for device rule changes to take effect."
	fi
	if [ $NO_RULES == $TRUE ]; then
		echo
		echo "No udev rules directory found."
		echo "Searched for $RULES_DEST_PRIMARY, $RULES_DEST_ALTERNATE."
		echo "Please copy $RULES to your device rules directory and reload the rules"
		echo "or contact LabJack support for assistance: <$SUPPORT_EMAIL>"
		let e=e+$NO_RULES_ERR
	fi
	if [ $NEED_RESTART == $TRUE ]; then
		echo
		echo "Please manually restart the device rules or restart your computer now."
	elif [ $NEED_RELOG == $TRUE ]; then
		echo
		echo "Please log off and log back in for the group changes to take effect."
		echo "To confirm the group changes have taken effect, enter the command:"
		echo "  $ groups"
		echo "and make sure $GROUP is in the list. (You probably have to log out"
		echo "of your entire account, not just your shell.)"
	fi
	exit $e
}

go ()
{
	$@
	ret=$?
	if [ $ret -ne 0 ]; then
		echo "Failure on command: $@"
		echo "Please contact LabJack at $SUPPORT_EMAIL"
		echo "Exiting"
		exit $ret
	fi
}

install_ljm_lib ()
{
	echo -n "Installing ${LJM_REALNAME} to ${LIB_DESTINATION}... "
	go install labjack_ljm/${LJM_REALNAME} ${LIB_DESTINATION}
	if [ -f ${LIB_DESTINATION}/${LJM_LINKERNAME} ]; then
		go rm -f ${LIB_DESTINATION}/${LJM_LINKERNAME}
	fi
	if [ -f ${LIB_DESTINATION}/${LJM_SONAME} ]; then
		go rm -f ${LIB_DESTINATION}/${LJM_SONAME}
	fi

	# Link
	go ln -s -f ${LIB_DESTINATION}/${LJM_REALNAME} ${LIB_DESTINATION}/${LJM_SONAME}
	go ln -s -f ${LIB_DESTINATION}/${LJM_SONAME} ${LIB_DESTINATION}/${LJM_LINKERNAME}
	echo "done."
}

install_ljm_header ()
{
	echo -n "Installing ${LJM_HEADER} to ${HEADER_DESTINATION}... "
	go install labjack_ljm/${LJM_HEADER} ${HEADER_DESTINATION}
	echo "done."
}

# Remove ljm_special_addresses.config or move it to ljm_specific_ips.config
deprecate_special_addresses()
{
    if [ -f "${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_special_addresses.config" ]; then
        if [ `cat ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_specific_ips.config | wc -c` -gt 1 ]; then
            echo "[Deprecation warning] Removing deprecated file:"
            echo "        ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_special_addresses.config"
            echo "    Contents were:"
            cat ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_special_addresses.config
            echo
            echo
            rm -f ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_special_addresses.config
        else
            echo "[Deprecation warning] Moving:"
            echo "        ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_special_addresses.config"
            echo "    to:"
            echo "        ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_specific_ips.config"
            mv -f \
                ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_special_addresses.config \
                ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_specific_ips.config
        fi
    fi
}

install_ljm_constants ()
{
	echo -n "Installing constants files to ${CONSTANTS_DESTINATION}... "
	if [ ! -d ${CONSTANTS_DESTINATION}/LabJack/LJM ]; then
		go mkdir --mode=777 -p ${CONSTANTS_DESTINATION}/LabJack
		go mkdir --mode=777 -p ${CONSTANTS_DESTINATION}/LabJack/LJM
	fi
	chmod 777 ${CONSTANTS_DESTINATION}/LabJack
	chmod 777 ${CONSTANTS_DESTINATION}/LabJack/LJM

	go install labjack_ljm/LabJack/LJM/ljm_constants.json \
        -t ${CONSTANTS_DESTINATION}/LabJack/LJM
	go install labjack_ljm/LabJack/LJM/ljm_startup_configs.json \
        -t ${CONSTANTS_DESTINATION}/LabJack/LJM
	go install labjack_ljm/LabJack/LJM/ljm.log \
        -t ${CONSTANTS_DESTINATION}/LabJack/LJM
	go install labjack_ljm/LabJack/LJM/readme.md \
        -t ${CONSTANTS_DESTINATION}/LabJack/LJM

    if [ ! -f ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_specific_ips.config ]; then
        go touch ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_specific_ips.config
    fi

	go chmod 666 ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_constants.json
	go chmod 666 ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_startup_configs.json
	go chmod 666 ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm.log
	go chmod 666 ${CONSTANTS_DESTINATION}/LabJack/LJM/ljm_specific_ips.config

	# Sic ("addressess")
	rm -f ${CONSTANTS_DESTINATION}/LabJack/LJM/readme_ljm_special_addressess.md

	deprecate_special_addresses

	echo "done."
}

install_exodriver_lib ()
{
	echo -n "Installing ${LIB_LABJACK_USB_VERSIONED} (Exodriver) to ${LIB_DESTINATION}... "
	go install labjack_ljm/${LIB_LABJACK_USB_VERSIONED} ${LIB_DESTINATION}
	go rm -f ${LIB_DESTINATION}/${LIB_LABJACK_USB}
	go ln -s -f ${LIB_DESTINATION}/${LIB_LABJACK_USB_VERSIONED} ${LIB_DESTINATION}/${LIB_LABJACK_USB}
	echo "done."
}

install_kipling ()
{
	echo -n "Installing ${KIPLING_INSTALL_DIR} to ${PROGRAM_DESTINATION}... "
	go cp --recursive ${KIPLING_INSTALL_DIR} ${PROGRAM_DESTINATION}
	echo "done."
	echo -n "Installing command line short-cut ${KIPLING_BIN_SHORTCUT} to ${BIN_DESTINATION}... "
	go install ${KIPLING_INSTALL_DIR}/${KIPLING_BIN_SHORTCUT} ${BIN_DESTINATION}
	echo "done."
}

setup_ldconfig ()
{
	path_exists=$FALSE
	for line in `cat $LDCONFIG_FILE`; do
		if [ $line == $LIB_DESTINATION ]; then
			path_exists=$TRUE
		fi
	done

	if [ $path_exists != $TRUE ]; then
		echo "$LIB_DESTINATION >> $LDCONFIG_FILE"
		echo $LIB_DESTINATION >> $LDCONFIG_FILE
	fi
	go ldconfig
}

setup_labjack_device_rules ()
{
	# LabJack device rules
	if [ -d $RULES_DEST_PRIMARY ]; then
		RULES_DEST=$RULES_DEST_PRIMARY
	elif [ -d $RULES_DEST_ALTERNATE ]; then
		RULES_DEST=$RULES_DEST_ALTERNATE
	else
		NO_RULES=$TRUE
	fi

	echo -n "Adding LabJack device rules... "
	if [ $NO_RULES != $TRUE ]; then
		if [ -f $RULES_DEST_ALTERNATE/$OLD_RULES ]; then
			#echo "Removing old rules: $RULES_DEST_ALTERNATE/$OLD_RULES.."
			go rm $RULES_DEST_ALTERNATE/$OLD_RULES
		fi

		#echo "Adding $RULES to $RULES_DEST.."
		go cp -f labjack_ljm/$RULES $RULES_DEST
		NEED_RECONNECT=$TRUE
	fi
	echo "done."
}

restart_device_rules ()
{
	echo -n "restarting the device rules... "
	udevadm control --reload-rules 2> /dev/null
	ret=$?
	if [ ! $ret ]; then
		udevadm control --reload_rules 2> /dev/null
		ret=$?
	fi
	if [ ! $ret ]; then
		/etc/init.d/udev-post reload 2> /dev/null
		ret=$?
	fi
	if [ ! $ret ]; then
		udevstart 2> /dev/null
		ret=$?
	fi
	if [ ! $ret ]; then
		NEED_RESTART=$TRUE
		echo " could not restart the rules."
	else
		echo "done."
	fi
}

add_user_to_group_if_needed ()
{
	user=$1
	group=$2

	if [ $USER_REALLY_IS_ROOT == $TRUE ]; then
		echo "root does not need to be added to '$group'"
		return
	fi

	in_group=$FALSE
	for g in `id -nG $user`; do
		if [ "$g" == "$group" ]; then
			in_group=$TRUE
			break
		fi
	done

	if [ $in_group == $TRUE ]; then
		# Make sure the user is logged into the group
		current_groups=$FALSE
		for g in `groups $user`; do
			if [ "$g" == "$group" ]; then
				current_groups=$TRUE
				break
			fi
		done
		if [ $current_groups != $TRUE ]; then
			NEED_RELOG=$TRUE
		fi

		echo "'$user' is already a member of '$group'"
		return
	fi

	echo -n "Adding current user '$user' to the '$group' group... "
	go groupadd -f $group
	go usermod -a -G $group $user
	NEED_RELOG=$TRUE
	echo " done."
}

# Some setup

VERSION_REGEX="^[0-9]+\.[0-9]+\.[0-9]+$"
if [[ ! $LJM_VERSION =~ $VERSION_REGEX ]] ; then
	echo "Argument $1 does not look like a version number, exiting."
	echo "Please contact LabJack at $SUPPORT_EMAIL"
	echo "Exiting"
	exit 1
fi

user=$USER
if [ -z "$user" ]; then
	user=`logname`
fi
if [ $user == "root" ]; then
	if [ -z "${SUDO_USER:-}" ]; then
		USER_REALLY_IS_ROOT=$TRUE
	else
		user=$SUDO_USER
	fi
fi
if [ -z "$user" ]; then
	echo "Unable to deduce the username."
	echo "You may need to manually set permissions for LabJack files."
	SKIP_USER_PERMISSIONS=$TRUE
fi

echo "Installing as user '$user'"


# Begin installing/configuring

install_ljm_lib
install_ljm_header
install_ljm_constants
install_exodriver_lib
install_kipling

# ldconfig setup, now that exodriver and labjack_ljm are installed
setup_ldconfig

setup_labjack_device_rules
restart_device_rules

if [ $SKIP_USER_PERMISSIONS == $FALSE ]; then
	add_user_to_group_if_needed $user $GROUP
fi

success
