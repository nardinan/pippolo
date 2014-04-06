/*
   Pippolo - a nosql distributed database.
   Copyright (C) 2012 Andrea Nardinocchi (nardinocchi@psychogames.net)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
   */
#include "xml.h"
str_token_node *p_xml_tokenizer (char *source) {
	struct str_token_node *result = NULL, *singleton = NULL;
	struct str_token_flags flags;
	struct str_token_controller controller;
	char *destination = source;
	int index, consistence;
	if ((source)) {
		flags.character.value = flags.safe.value = flags.special.value = flags.symbol.value = flags.interrupt.value = pippolo_false;
		controller.closed.value = controller.opened.value = pippolo_false;
		while (*destination) {
			/* safe characters */
			if (((*destination >= 32) && (*destination <= 126)) || (*destination == 9) || (*destination == 10)) {
				flags.interrupt.value = pippolo_false;
				switch (*destination) {
					case '?':
					case '/':
					case '=':
						if (controller.opened.value) {
							flags.symbol.value = pippolo_true;
							flags.character.value = pippolo_false;
						} else {
							flags.character.value = pippolo_true;
							flags.symbol.value = pippolo_false;
						}
						flags.safe.value = pippolo_false;
						break;
					case '<':
						controller.opened.value = pippolo_true;
						controller.closed.value = pippolo_false;
						flags.symbol.value = pippolo_true;
						flags.character.value = flags.safe.value = pippolo_false;
						flags.interrupt.value = pippolo_true;
						break;
					case '>':
						controller.closed.value = pippolo_true;
						controller.opened.value = pippolo_false;
						flags.symbol.value = pippolo_true;
						flags.character.value = flags.safe.value = pippolo_false;
						flags.interrupt.value = pippolo_true;
						break;
					case '\\':
						flags.safe.value = flags.special.value = pippolo_false;
						break;
					case ' ':
					case '\t':
						flags.special.value = pippolo_true;
						flags.symbol.value = flags.character.value = flags.safe.value = pippolo_false;
						break;
					default:
						flags.symbol.value = flags.safe.value = pippolo_false;
						flags.character.value = pippolo_true;
				}
				if (singleton) {
					if ((singleton->flags.character.value != flags.character.value) ||
							(singleton->flags.special.value != flags.special.value) ||
							(singleton->flags.symbol.value != flags.symbol.value) ||
							(flags.interrupt.value)) {
						index = 0;
						consistence = pippolo_false;
						while (index < singleton->length) {
							if (p_check_consistence(singleton->token[index])) {
								consistence = pippolo_true;
								break;
							}
							index++;
						}
						if (consistence) {
							/* alloc the next node and shift singleton */
							if ((singleton->next = (struct str_token_node *) pippolo_malloc(sizeof(str_token_node))))
								singleton = singleton->next;
							else
								pippolo_kill("out of memory, I'm dying bleaaargh");
						}
						memcpy(&singleton->flags, &flags, sizeof(struct str_token_flags));
						memset(singleton->token, '\0', pippolo_data);
						singleton->next = NULL;
						singleton->length = 0;
					}
					singleton->token[singleton->length++] = *destination;
				} else if ((singleton = (struct str_token_node *) pippolo_malloc(sizeof(str_token_node)))) {
					memcpy(&singleton->flags, &flags, sizeof(struct str_token_flags));
					memset(singleton->token, '\0', pippolo_data);
					singleton->next = NULL;
					singleton->length = 0;
					singleton->token[singleton->length++] = *destination;
					result = singleton;
				} else
					pippolo_kill("out of memory, I'm dying bleaaargh");
			}
			destination++;
		}
		if (singleton) {
			index = 0;
			consistence = pippolo_false;
			while (index < singleton->length)
				if (p_check_consistence(singleton->token[index++])) {
					consistence = pippolo_true;
					break;
				}
			if (!consistence) {
				memcpy(&singleton->flags, &flags, sizeof(struct str_token_flags));
				memset(singleton->token, '\0', pippolo_data);
				singleton->next = NULL;
				singleton->length = 0;
			}
		}
	}
	return result;
}

int p_xml_key_append (struct str_xml_key **record, char *key, char *value) {
	struct str_xml_key *singleton;
	size_t length;
	int result = pippolo_true;
	p_string_trim(key);
	p_string_trim(value);
	if ((key) && (value)) {
		if ((singleton = (struct str_xml_key *) pippolo_malloc(sizeof(str_xml_key)))) {
			length = p_string_len(value)+1;
			if ((singleton->value = (char *) pippolo_malloc(length))) {
				snprintf(singleton->value, length, "%s", value);
				length = p_string_len(key)+1;
				if ((singleton->key = (char *) pippolo_malloc(length))) {
					snprintf(singleton->key, length, "%s", key);
					singleton->next = *record;
					*record = singleton;
				} else
					pippolo_kill("out of memory, I'm dying bleaaargh");
			} else
				pippolo_kill("out of memory, I'm dying bleaaargh");
		} else
			pippolo_kill("out of memory, I'm dying bleaaargh");
	}
	return result;
}

int p_xml_analyze (struct str_xml_node **root, str_token_node *node) {
	int result = pippolo_true;
	if ((result = _p_xml_analyze (root, node, EXML_ACTIONS_READ)))
		result = p_xml_data_trimming(*root);
	return result;
}

int _p_xml_analyze (struct str_xml_node **root, str_token_node *node, enum enum_xml_action action) {
	struct str_xml_node *current = *root, *singleton;
	struct str_token_node *key = NULL;
	int result = pippolo_true, passed;
	size_t length;
	enum enum_xml_action next = action;
	while (node) {
		passed = pippolo_false;
		if (node->token[0] == '<') {
			if (node->length > 1) {
				if ((node->token[1] == '/') || (node->token[1] == '?')) {
					next = EXML_ACTIONS_JUMP;
					passed = pippolo_true;
				}
			} else {
				next = EXML_ACTIONS_TITLE;
				passed = pippolo_true;
			}
		} else if (node->token[0] == '/') {
			if (node->length > 1) {
				if (node->token[1] == '>') {
					if (current)
						current = current->father;
					next = EXML_ACTIONS_CVALUE;
					passed = pippolo_true;
				}
			}
		} else if (node->token[0] == '>') {
			if (next == EXML_ACTIONS_JUMP)
				if (current)
					current = current->father;
			next = EXML_ACTIONS_CVALUE;
			passed = pippolo_true;
		}
		if ((!passed) && (action != EXML_ACTIONS_JUMP)) {
			switch (next) {
				case EXML_ACTIONS_TITLE:
					if (p_xml_node_allocate(&singleton)) {
						singleton->father = current;
						length = p_string_len(node->token)+1;
						if ((singleton->label = (char *) pippolo_malloc(length))) {
							snprintf(singleton->label, length, "%s", node->token);
							if ((*root)) {
								if (current) {
									if (current->children)
										singleton->next = current->children;
									current->children = singleton;
								}
							} else
								*root = singleton;
							current = singleton;
						} else
							pippolo_kill("out of memory, I'm dying bleaaargh");
					} else
						pippolo_kill("out of memory, I'm dying bleaaargh");
					next = EXML_ACTIONS_KEY;
					break;
				case EXML_ACTIONS_KEY:
					key = node;
					next = EXML_ACTIONS_KVALUE;
					break;
				case EXML_ACTIONS_KVALUE:
					if ((current) && (key)) {
						if (p_string_cmp(node->token, "=") != 0) {
							result = p_xml_key_append(&(current->keys), key->token, node->token);
							next = EXML_ACTIONS_KEY;
							key = NULL;
						}
					}
					break;
				case EXML_ACTIONS_CVALUE:
					if (current)
						pippolo_append(current->value, node->token);
				default:
					break;

			}
		}
		node = node->next;
	}
	return result;
}

int p_xml_duplicate_tree (struct str_xml_node **hook, struct str_xml_node *father, struct str_xml_node *source) { /* partially avoiding recursivity */
	int result = pippolo_true;
	struct str_xml_key *key;
	if (source) {
		if ((result = p_xml_node_allocate(hook))) {
			(*hook)->father = father;
			if (source->label)
				pippolo_append((*hook)->label, source->label);
			if (source->value)
				pippolo_append((*hook)->value, source->value);
			key = source->keys;
			while(key) {
				if (!(result = p_xml_key_append(&((*hook)->keys), key->key, key->value)))
					break;
				key = key->next;
			}
			if ((result = p_xml_duplicate_tree(&((*hook)->children), (*hook), source->children)))
				result = p_xml_duplicate_tree(&((*hook)->next), father, source->next);
		}
	}
	return result;
}

int p_xml_node_allocate (struct str_xml_node **hook) {
	int result = pippolo_true;
	if (((*hook) = (struct str_xml_node *) pippolo_malloc(sizeof(str_xml_node)))) {
		(*hook)->compiled.value = pippolo_false;
		(*hook)->father = (*hook)->next = (*hook)->children = NULL;
		(*hook)->label = (*hook)->value = NULL;
		(*hook)->keys = NULL;
	} else
		pippolo_kill("out of memory, I'm dying bleaaargh");
	return result;
}

int p_xml_data_trimming (str_xml_node *root) {
	int result = pippolo_true;
	if (root) {
		if (root->value)
			p_string_trim(root->value);
		if ((result = p_xml_data_trimming(root->next)))
			result = p_xml_data_trimming(root->children);        
	}
	return result;
}

int p_xml_draw (FILE *stream, str_xml_node *root) {
	return _p_xml_draw(stream, root, 0);
}

int _p_xml_draw (FILE *stream, str_xml_node *root, int deep) {
	int result = pippolo_true, index;
	struct str_xml_key *backup;
	struct str_xml_node *childrens;
	if (root) {
		if (p_string_len(root->label) > 0) {
			for (index = 0; index < deep; index++)
				fprintf(stream, "\t");
			fprintf(stream, "<%s", root->label);
			backup = root->keys;
			while (backup) {
				fprintf(stream, " %s=%s", backup->key, backup->value);
				backup = backup->next;
			}
			fprintf(stream, ">");
		}
		if (root->value)
			fprintf(stream, "%s", root->value);
		if (root->children) {
			childrens = root->children;
			fprintf(stream, "\n");
			while (childrens) {
				if (!(result = _p_xml_draw(stream, childrens, deep+1)))
					break;
				childrens = childrens->next;
			}
		}
		if (p_string_len(root->label) > 0) {
			if ((root->children))
				for (index = 0; index < deep; index++)
					fprintf(stream, "\t");
			fprintf(stream, "</%s>", root->label);
		}
	}
	fprintf(stream, "\n");
	return result;
}

int p_xml_string (char **string, str_xml_node *root) {
	return _p_xml_string(string, root);
}

int _p_xml_string (char **string, str_xml_node *root) {
	int result = pippolo_true;
	struct str_xml_key *backup;
	struct str_xml_node *childrens;
	if (root) {
		if (p_string_len(root->label) > 0) {
			pippolo_append(*string, "<");
			pippolo_append(*string, root->label);
			backup = root->keys;
			while (backup) {
				pippolo_append(*string, " ");
				pippolo_append(*string, backup->key);
				pippolo_append(*string, "=");
				pippolo_append(*string, backup->value);
				backup = backup->next;
			}
			pippolo_append(*string, ">");
		}
		if (root->value)
			pippolo_append(*string, root->value);
		if (root->children) {
			childrens = root->children;
			while (childrens) {
				if (!(result = _p_xml_string(string, childrens)))
					break;
				childrens = childrens->next;
			}
		}
		if (p_string_len(root->label) > 0) {
			pippolo_append(*string, "</");
			pippolo_append(*string, root->label);
			pippolo_append(*string, ">");
		}
	}
	return result;
}

void p_tokens_free (struct str_token_node **root) { /* avoiding recursivity */
	struct str_token_node *support;
	if ((support = *root)) {
		*root = (*root)->next;
		pippolo_null_free(support);
	}
}

void p_xml_free (struct str_xml_node **root) {
	if ((*root)) {
		if ((*root)->value)
			pippolo_free((*root)->value);
		if ((*root)->label)
			pippolo_free((*root)->label);
		if ((*root)->compiled.value)
			regfree(&((*root)->regex));
		p_xml_free(&(*root)->next);
		p_xml_free(&(*root)->children);
		p_xml_key_free(&((*root)->keys));
		pippolo_null_free((*root));
	}
}

void p_xml_node_free (struct str_xml_node **root) {
	if ((*root)) {
		if ((*root)->value)
			pippolo_free((*root)->value);
		if ((*root)->label)
			pippolo_free((*root)->label);
		if ((*root)->compiled.value)
			regfree(&((*root)->regex));
		p_xml_free(&(*root)->children);
		p_xml_key_free(&((*root)->keys));
		pippolo_null_free((*root));
	}
}

void p_xml_key_free (struct str_xml_key **root) {
	if ((*root)) {
		if ((*root)->key)
			pippolo_free((*root)->key);
		if ((*root)->value)
			pippolo_free((*root)->value);
		p_xml_key_free(&((*root)->next));
		pippolo_null_free((*root));
	}
}
