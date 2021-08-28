/*
 * graftcp
 * Copyright (C) 2021 Hmgle <dustgle@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "conf.h"

static int config_local_addr(const char *, const char *, struct graftcp_conf *);
static int config_local_port(const char *, const char *, struct graftcp_conf *);
static int config_pipe_path(const char *, const char *, struct graftcp_conf *);
static int config_blackip_file_path(const char *, const char *, struct graftcp_conf *);
static int config_whiteip_file_path(const char *, const char *, struct graftcp_conf *);
static int config_ignore_local(const char *, const char *, struct graftcp_conf *);

static struct graftcp_config_t config[] = {
	{ "local_addr",        config_local_addr        },
	{ "local_port",        config_local_port        },
	{ "pipepath",          config_pipe_path         },
	{ "blackip_file_path", config_blackip_file_path },
	{ "whiteip_file_path", config_whiteip_file_path },
	{ "ignore_local",      config_ignore_local      },
};

static int config_local_addr(const char *key, const char *value, struct graftcp_conf *conf)
{
	// TODO: check value
	conf->local_addr = strdup(value);
	return 0;
}

static int config_local_port(const char *key, const char *value, struct graftcp_conf *conf)
{
	// TODO: check value
	conf->local_port = malloc(*conf->local_port);
	*conf->local_port = atoi(value);
	return 0;
}

static int config_pipe_path(const char *key, const char *value, struct graftcp_conf *conf)
{
	// TODO: check value
	conf->pipe_path = strdup(value);
	return 0;
}

static int config_blackip_file_path(const char *key, const char *value, struct graftcp_conf *conf)
{
	// TODO: check value
	conf->blackip_file_path = strdup(value);
	return 0;
}

static int config_whiteip_file_path(const char *key, const char *value, struct graftcp_conf *conf)
{
	// TODO: check value
	conf->whiteip_file_path = strdup(value);
	return 0;
}

static int config_ignore_local(const char *key, const char *value, struct graftcp_conf *conf)
{
	conf->ignore_local = malloc(*conf->ignore_local);
	// TODO: check value
	if (strcmp(value, "true") || strcmp(value, "1")) {
		*conf->ignore_local = true;
	} else {
		*conf->ignore_local = false;
	}
	return 0;
}

static const size_t config_size = sizeof(config) / sizeof(struct graftcp_config_t);

static struct graftcp_config_t *graftcp_getconfig(const char *key)
{
	int i;

	for (i = 0; i < config_size; i++)
		if (!strncmp(config[i].name, key, strlen(config[i].name)))
			return &config[i];
	return NULL;
}

static int is_line_empty(char *line)
{
	int i;
	size_t len = strlen(line);

	for (i = 0; i < len; i++)
		if (!isspace(line[i]))
			return 0;
	return 1;
}

static int left_space(char *buf, size_t len)
{
	int i;
	for (i = 0; i < len; i++)
		if (buf[i] != ' ' && buf[i] != '\t')
			return i;
	return i;
}

static int right_space(char *buf, size_t len)
{
	int i;
	for (i = len - 1; i >= 0; i--)
		if (buf[i] != ' ' && buf[i] != '\t' && buf[i] != '\0')
			return i;
	return 0;
}

static int parse_line(char *buf, struct graftcp_conf *conf)
{
	char *key;
	char *value;
	char *fs;
	struct graftcp_config_t *config;

	if (is_line_empty(buf))
		return 0;
	buf += left_space(buf, strlen(buf));
	if (buf[0] == '#')
		return 0;

	fs = strstr(buf, "=");
	if (!fs)
		return -1;

	*fs = '\0';
	value = fs + 1;

	key = buf;
	key[right_space(key, strlen(key))] = '\0';

	value += left_space(value, strlen(value));
	value[right_space(value, strlen(value))] = '\0';

	config = graftcp_getconfig(key);
	if (!config) {
		fprintf(stderr, "unknown key %s", key);
		return -1;
	}

	return config->cb(key, value, conf);
}

int conf_init(struct graftcp_conf *conf)
{
	conf->local_addr = NULL;
	conf->local_port = NULL;
	conf->pipe_path = NULL;
	conf->blackip_file_path = NULL;
	conf->whiteip_file_path = NULL;
	conf->ignore_local = NULL;
	return 0;
}

int conf_read(const char *path, struct graftcp_conf *conf)
{
	FILE *f;
	char *line = NULL;
	size_t len = 0;
	int err = 0;

	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Failed to open %s\n", path);
		return -1;
	}

	while (getline(&line, &len, f) != -1) {
		err = parse_line(line, conf);
		if (err) {
			fprintf(stderr, "Failed to parse config: %s\n", line);
			break;
		}
	}
	free(line);
	fclose(f);
	return err;
}
