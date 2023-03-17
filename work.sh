#! /bin/bash

leave()
{
	func_name=(help cleanAll killNode killAll runNode debug leave)
	for func in ${func_name};do
		unset -f ${func}
	done
}

help()
{
	echo \
'Usage: source work.sh <command>

Command:
	kill [datadir]
		kill node whose -datadir=[datadir], or default if [datadir] is empty

	killall
		kill all bitcoin node

	clean
		do first, when you want to git push

	run [datadir]
		run node whose -datadir=[datadir], or defualt if [datadir] is empty
	'
}

cleanAll()
{
	find ./1/* -not -name 'bitcoin.conf' -delete
	find ./2/* -not -name 'bitcoin.conf' -delete
	find ./oracle/* -not -name 'bitcoin.conf' -delete
	rm *.txt src/privacy/{proof.json,witness,*.sol,*.c} >/dev/null 2>&1
	rm src/ourcontract-rt
	rm .vs* -rf
}

killNode()
{
	if [ $# = 0 ];then
		pid=`ps aux | grep bitcoind | awk '{print $2, $12;}' | grep -E -v 'datadir|color' | cut -d ' ' -f 1`
		kill -9 ${pid}
	else
		all_match=`ps aux | grep bitcoind | awk '{print $2, $12;}' | grep 'datadir' | sed 's/-datadir=/ /g'`
		all_match=(${all_match[@]})
		for ((i=0; i<${#all_match[@]} ;i+=2));do
			if [ ${all_match[i+1]} = ${1} ];then
				kill -9 ${all_match[i]}
			fi
		done
	fi
}

killAll()
{
	killall bitcoind
}

runNode()
{
	if [ $# = 0 ];then
		find  ~/.bitcoin/* -not -name 'bitcoin.conf' -delete
		bitcoind -regtest -txindex -reindex -daemon
		mkdir ~/.bitcoin/regtest/contracts -p && cp 0000* ~/.bitcoin/regtest/contracts/ -r
		mkdir ~/.bitcoin/regtest/privacy -p
	else 
		alias b${1:0:1}="bitcoin-cli -datadir=${1}"
		find ./${1}/* -not -name 'bitcoin.conf' -delete
		if [ $? = 1 ];then
			return 1
		fi

		bitcoind -datadir=${1} -daemon
		mkdir $1/regtest/contracts -p &&cp 0000* ${1}/regtest/contracts/ -r
		mkdir $1/regtest/privacy -p
	fi
}

debug()
{	# debug my work 
	b1 generate 2
	FROM=$(b1 getnewaddress)
	sleep 1
	TO=$(b2 getnewaddress)
	sleep 1
	#privkey1=$(b1 dumpprivkey $(b1 getaccountaddress ""))
	#sleep 1
	#privkey2=$(b2 dumpprivkey $(b2 getaccountaddress ""))
	#echo $privkey1
	#sleep 1
	#echo $privkey2
	#./autofill.sh 2>&1
}

if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then
  emulate bash
fi


if [ $# -lt 1 -o "${1}" = "help" -o "${1}" = "-h" ];then
	help && leave
	return 0
fi

case "${1}" in
	"kill")
		killNode ${2} ;;
	"killall")
		killAll ;;
	"run")
		runNode ${2} ;;
	"clean")
		cleanAll ;;
	"debug")
		debug ;;
esac

leave

if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then
  emulate zsh
fi
