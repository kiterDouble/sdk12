/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "zlog.h"

int main(int argc, char **argv)
{
	int rc;

	rc = dzlog_init("test.conf", "my_cat");

	if (rc)
	{

		printf("init failed\n");

		return -1;
	}

	int i = 1000000;
	while(i--)
	{
		dzlog_info("hello, zlog %d", i);
		// usleep(100000);
	}
	zlog_fini();

	return 0;
}
