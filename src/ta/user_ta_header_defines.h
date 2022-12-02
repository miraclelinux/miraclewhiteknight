/*
 * Copyright (C) 2018,2019 Cybertrust Japan Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#pragma once

/* Operations */
#define TEE_WL_APPEND		0x01
#define TEE_WL_REMOVE		0x02
#define TEE_WL_FLUSH		0x03
#define TEE_WL_VERIFY		0x04
#define TEE_ML_ADD			0x05
#define TEE_ML_FLUSH		0x06
#define TEE_ML_REMOVE		0x07
#define TEE_ML_GET_LIST		0x08
#define TEE_CH_MGTMODE		0x09
#define TEE_KEY_REGIST		0x10
#define TEE_KEY_UNREGIST	0x11
#define TEE_GET_OBJSIZE		0x12
#define TEE_WL_INIT			0x13
#define TEE_KEY_LIST		0x14
#define TEE_KEY_GEN			0x15
#define TEE_KEY_GET_PUB		0x16

/* Errors */
#define TEE_ACCESS_GRANTED			0x00000000
#define TEE_ERROR_NO_ENTRY			0xFFFF1001
#define TEE_ERROR_INVALID_HASH		0xFFFF1002

#define TEEC_ACCESS_GRANTED			0x00000000
#define TEEC_ERROR_NO_ENTRY			0xFFFF1001
#define TEEC_ERROR_INVALID_HASH		0xFFFF1002

/* 8189b937-e77d-417f-9114-9dda34f8683a */
#define TA_UUID	{ 0x8189b937, 0xe77d, 0x417f, { 0x91, 0x14, 0x9d, 0xda, 0x34, 0xf8, 0x68, 0x3a } }

#define TA_FLAGS	(TA_FLAG_SINGLE_INSTANCE | TA_FLAG_MULTI_SESSION | TA_FLAG_EXEC_DDR)

#define TA_STACK_SIZE	(2 * 1024 * 1024)
#define TA_DATA_SIZE	(5 * 1024 * 1024)
