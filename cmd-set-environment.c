/* $OpenBSD$ */

/*
 * Copyright (c) 2009 Nicholas Marriott <nicholas.marriott@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

#include "tmux.h"

/*
 * Set an environment variable.
 */

static enum cmd_retval	cmd_set_environment_exec(struct cmd *,
			    struct cmdq_item *);

const struct cmd_entry cmd_set_environment_entry = {
	.name = "set-environment",
	.alias = "setenv",

	.args = { "hgrt:u", 1, 2 },
	.usage = "[-hgru] " CMD_TARGET_SESSION_USAGE " name [value]",

	.target = { 't', CMD_FIND_SESSION, CMD_FIND_CANFAIL },

	.flags = CMD_AFTERHOOK,
	.exec = cmd_set_environment_exec
};

static enum cmd_retval
cmd_set_environment_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args	*args = cmd_get_args(self);
	struct environ	*env;
	const char	*name, *value, *target;

	name = args->argv[0];
	if (*name == '\0') {
		cmdq_error(item, "empty variable name");
		return (CMD_RETURN_ERROR);
	}
	if (strchr(name, '=') != NULL) {
		cmdq_error(item, "variable name contains =");
		return (CMD_RETURN_ERROR);
	}

	if (args->argc < 2)
		value = NULL;
	else
		value = args->argv[1];

	if (args_has(args, 'g'))
		env = global_environ;
	else {
		if (item->target.s == NULL) {
			target = args_get(args, 't');
			if (target != NULL)
				cmdq_error(item, "no such session: %s", target);
			else
				cmdq_error(item, "no current session");
			return (CMD_RETURN_ERROR);
		}
		env = item->target.s->environ;
	}

	if (args_has(args, 'u')) {
		if (value != NULL) {
			cmdq_error(item, "can't specify a value with -u");
			return (CMD_RETURN_ERROR);
		}
		environ_unset(env, name);
	} else if (args_has(args, 'r')) {
		if (value != NULL) {
			cmdq_error(item, "can't specify a value with -r");
			return (CMD_RETURN_ERROR);
		}
		environ_clear(env, name);
	} else {
		if (value == NULL) {
			cmdq_error(item, "no value specified");
			return (CMD_RETURN_ERROR);
		}
		if (args_has(args, 'h'))
			environ_set(env, name, ENVIRON_HIDDEN, "%s", value);
		else
			environ_set(env, name, 0, "%s", value);
	}

	return (CMD_RETURN_NORMAL);
}
