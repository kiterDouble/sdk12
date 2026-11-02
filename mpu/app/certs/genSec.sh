#!/bin/bash
current_path=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
SEC_PATH=${current_path}/ext/Linux
OUT_PATH=${current_path}/output
SEC_DATA=${OUT_PATH}/Secdata
#OEM_HASH="0xa01d0af3bd078757631f083ba3a88572aae52996f9232f42ed85b483e77a6a09f06e2a090a871ebf22452dd4a44402ff"
OEM_HASH="0x9134ac2984d0033e9a531e684906ae1b541c4443fc4a10bbcc9e3e6af0d82371c572be22cd58f19a7a20e79dbd999366"

function secboot_pro_commerical(){
	${SEC_PATH}/sectools fuse-blower --generate --security-profile ./configs/pinnacles_security_profile.xml \
	--fuse-enforce-oem-authz-on-soc-debug --fuse-enforce-oem-authz-on-fuse-overrides --fuse-enforce-oem-authz-on-rom-patch \
	--fuse-activate-security-policy --fuse-auth-en --fuse-pk-hash-in-fuse --fuse-oem-security-policy-write-disable \
	--fuse-write-permissions-write-disable --fuse-read-permissions-write-disable --fuse-mrc-hash-write-disable \
	--fuse-fuse-redundancy-enable-write-disable --fuse-debug-disable-write-disable --fuse-oem-config-write-disable \
	--fuse-oem-product-seed-write-disable --fuse-oem-product-seed-read-disable --fuse-mrc-hash-fec-enable \
	--fuse-oem-product-seed-fec-enable --fuse-qsee-dbgen-disable --fuse-qsee-niden-disable --fuse-apps-dbgen-disable \
	--fuse-apps-niden-disable --fuse-mss-scalar-niden-disable --fuse-ddrss-niden-disable --fuse-dap-dbgen-disable \
	--fuse-dap-niden-disable --fuse-apps-scandump-disable --fuse-ddrss-scandump-disable --fuse-debugbus-disable \
	--fuse-dcc-disable --fuse-smmu-scandump-disable --fuse-wdog-en --fuse-eku-enforcement-en --fuse-anti-rollback-feature-en=0xf \
	--fuse-oem-product-seed=0xc497ae9dd48c91ca0d0555bd91e7c11a --fuse-oem-device-lcs=0x3 --fuse-oem-hw-id=0x0159 --fuse-oem-product-id=0x0 \
	--oem-id=0x0159 --oem-product-id=0x0 --fuse-mrc-hash=${OEM_HASH} \
	--sign --signing-mode=LOCAL --root-certificate=./OEM-KEYS/certs/oem_rootca.cer --ca-certificate=./OEM-KEYS/certs/oem_attestca.cer \
	--ca-key=./OEM-KEYS/certs/oem_attestca.key --outfile ${SEC_DATA}/sec.elf
}

create_dir(){
	if [ ! -d "${SEC_DATA}" ]; then
		mkdir -p ${SEC_DATA}
	fi
}

main(){
	create_dir
	secboot_pro_commerical
}

main
