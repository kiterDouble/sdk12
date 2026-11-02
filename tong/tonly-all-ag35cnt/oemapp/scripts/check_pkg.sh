####################################################
#file Name: check_pkg.sh for GAC T08Y1
# Author: bcs
# Updated Time: 2025-10-15
#########################################################################
#!/bin/sh

pkg_dir=/sdcard/selfUpgrade/decompress
run_dir=$pkg_dir/run.sh
failnum=0

while true; do
	if [ -f $run_dir ]; then
		chmod +x $pkg_dir/*
		echo $run_dir
		$run_dir &
		exit 0
	fi
	
	if [[ -f $pkg_dir/*.tgz ]] && [[ -f $pkg_dir/key ]]; then
		cd $pkg_dir
		loc_md5=`md5sum *.tgz`
		key=${loc_md5:9:3}${loc_md5:5:2}${loc_md5:2:1}
		getkey=`cat $pkg_dir/key`
		echo $key
		if [ "$getkey" = "$key" ]; then
			tar zxvf $pkg_dir/*.tgz -C $pkg_dir
		else
			$failnum = $failnum + 1
			echo "check fail, wait for next round"
			if [ $failnum -ge 6 ]; then
				rm -rf $pkg_dir/*
				vin=`cat /oemdata/tmp/vin.txt`
				version=`cat /oemapp/version.txt`
				iccid=`cat /oemdata/tmp/iccid.txt`
				server_ip="47.97.113.255"
				sendstr="3#"$vin"#"$iccid"#"$version"#updateFail:check_pkg"
				timeout 6 /oemapp/apps/imok $server_ip $sendstr
			fi
		fi
		sync
	fi
	sleep 15
done
exit 0
