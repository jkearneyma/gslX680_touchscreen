# convert from the somewhat inefficient format (because of alignment) of the Silead firmware source to a byte-aligned one.

BEGIN {
	print "const uint8_t GSLX680_FW[] = {" 
	FS = "[{,}]"
	count = 0
}

/^{0x/ {
	addr = strtonum($2)
	bytes = strtonum($3)
	printf "  0x%02x, ", addr
	for (i = 0; i < 4; ++i) {
		printf " 0x%02x,", (bytes % 256)
		bytes /= 256
	}
	count += 5
	if (0 == count % 20) {
		printf "\n"
	}
}

END {
	printf "\n};\n\nconst size_t GSLX680_FW_bytes = %u;\n", count
}

