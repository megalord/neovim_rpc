#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmp.c"

FILE *out_h_file;
FILE *out_c_file;

bool file_reader (cmp_ctx_t *ctx, void *data, size_t limit) {
  return fread(data, sizeof(uint8_t), limit, ctx->buf) == (limit * sizeof(uint8_t));
}

typedef struct {
  char name[20];
  char type[20];
  bool is_arr;
  bool is_out;
  bool is_ptr;
  char cmp_fn[80];
} param_t;

typedef struct {
  char name[40];
  bool deprecated;
  param_t ret;
  uint32_t num_params;
  param_t *params;
} func_t;

size_t null_writer (cmp_ctx_t *ctx, const void *data, size_t count) {
  fprintf(stderr, "do not write to the output\n");
  exit(1);
}

void error_and_exit (const char str[]) {
  fprintf(stderr, "%s\n", str);
  exit(1);
}

void read_version (cmp_ctx_t *cmp) {
  uint32_t map_size;
  cmp_read_map(cmp, &map_size);

  uint8_t major, minor, patch, val;
  for (int i = 0; i < map_size; i++) {
    uint32_t key_size = 20;
    char key[key_size];
    if(!cmp_read_str(cmp, key, &key_size)) {
      error_and_exit(cmp_strerror(cmp));
    }

    cmp_object_t cmp_obj;
    if (!cmp_read_pfix(cmp, &val)) {
      if (cmp->error != INVALID_TYPE_ERROR) {
        error_and_exit(cmp_strerror(cmp));
      }
    }

    if (strcmp(key, "major") == 0) {
      major = val;
    } else if (strcmp(key, "minor") == 0) {
      minor = val;
    } else if (strcmp(key, "patch") == 0) {
      patch = val;
    }
  }

  fprintf(out_h_file, "\n// The definitions below are automatically generated for Neovim Version %i.%i.%i\n\n", major, minor, patch);
  fprintf(out_c_file, "\n// The definitions below are automatically generated for Neovim Version %i.%i.%i\n\n", major, minor, patch);
}

void print_param (param_t *param) {
  fprintf(out_h_file, "%s ", param->type);
  fprintf(out_c_file, "%s ", param->type);
  if (param->is_arr) {
    fprintf(out_h_file, "*");
    fprintf(out_c_file, "*");
  }
  if (param->is_out) {
    fprintf(out_h_file, "*");
    fprintf(out_c_file, "*");
  }
  if (param->is_ptr) {
    fprintf(out_h_file, "*");
    fprintf(out_c_file, "*");
  }
  fprintf(out_h_file, "%s", param->name);
  fprintf(out_c_file, "%s", param->name);

  // array params have an implicit size param
  if (param->is_arr) {
    fprintf(out_h_file, ", uint32_t ");
    fprintf(out_c_file, ", uint32_t ");
    if (param->is_out) {
      fprintf(out_h_file, "*");
      fprintf(out_c_file, "*");
    }
    fprintf(out_h_file, "%s_size", param->name);
    fprintf(out_c_file, "%s_size", param->name);
  }
}

void print_result_collector (param_t *p) {
  char result[40] = "";
  fprintf(out_c_file, "  if (!read_message_headers()) {\n    return false;\n  }\n");
  strcat(result, p->name);
  if (p->is_arr) {
    fprintf(out_c_file, "  if (!cmp_read_array(&cmp, %s_size)) {\n    return false;\n  }\n", p->name);
    fprintf(out_c_file, "  *%s = malloc(*%s_size * sizeof(%s *));\n", result, p->name, p->type);
    fprintf(out_c_file, "  for (int i = 0; i < *%s_size; i++) {\n", p->name);
    char tmp[40];
    strcpy(tmp, result);
    sprintf(result, "(*%s) + i", tmp);
  }
  if (strcmp(p->type, "void") == 0) {
    fprintf(out_c_file, "  if (!cmp_read_nil(&cmp)) {\n    return false;\n  }\n");
  } else if (strcmp(p->type, "char") == 0) {
    fprintf(out_c_file, "  if (!read_string(%s)) {\n    return false;\n  }\n", result);
  } else if (strcmp(p->type, "int64_t") == 0) {
    fprintf(out_c_file, "  if (!cmp_read_integer(&cmp, %s)) {\n    return false;\n  }\n", result);
  } else if (strcmp(p->type, "Buffer") == 0) {
    fprintf(out_c_file, "  int8_t ext_type;\n  uint32_t ext_size;\n");
    fprintf(out_c_file, "  if (!cmp_read_ext(&cmp, &ext_type, &ext_size, %s)) {\n    return false;\n  }\n", result);
  }
  if (p->is_arr) {
    fprintf(out_c_file, "  }\n");
  }
  fprintf(out_c_file, "  return true;\n");
}

void print_function (func_t *fn) {
  fprintf(out_h_file, "bool %s (", fn->name);
  fprintf(out_c_file, "bool %s (", fn->name);
  if (fn->num_params > 0) {
    print_param(&fn->params[0]);
    for (int i = 1; i < fn->num_params; i++) {
      fprintf(out_h_file, ", ");
      fprintf(out_c_file, ", ");
      print_param(&fn->params[i]);
    }
  }
  if (strcmp(fn->ret.type, "void") != 0 || fn->ret.is_ptr) {
    if (fn->num_params > 0) {
      fprintf(out_h_file, ", ");
      fprintf(out_c_file, ", ");
    }
    print_param(&fn->ret);
  }
  fprintf(out_h_file, ");\n");
  fprintf(out_c_file, ") {\n");
  fprintf(out_c_file, "  if (!rpc_send(NVIM_RPC_REQUEST, \"%s\", %i)) {\n    return false;\n  }\n", fn->name, fn->num_params);
  for (int i = 0; i < fn->num_params; i++) {
    if (fn->params[i].is_arr) {
      fprintf(out_c_file, "  if (!cmp_write_array(&cmp, %s_size)) {\n", fn->params[i].name);
      fprintf(out_c_file, "    return false;\n  }\n");
      fprintf(out_c_file, "  for (int i = 0; i < %s_size; i++) {\n", fn->params[i].name);
      fprintf(out_c_file, "    if (!%s) {\n      return false;\n    }\n", fn->params[i].cmp_fn);
      fprintf(out_c_file, "  }\n");
    } else {
      fprintf(out_c_file, "  if (!%s) {\n    return false;\n  }\n", fn->params[i].cmp_fn);
    }
  }
  print_result_collector(&fn->ret);
  fprintf(out_c_file, "}\n\n");
}

void translate_param (param_t *param) {
  // See https://github.com/neovim/neovim/blob/master/src/nvim/api/private/defs.h
  char type[30];
  strcpy(type, param->type);
  if (strcmp(type, "String") == 0) {
    strcpy(param->type, "char");
    param->is_ptr = true;
    snprintf(param->cmp_fn, 80, "cmp_write_str(&cmp, %s, strlen(%s))", param->name, param->name);
  } else if (strcmp(type, "Integer") == 0) {
    strcpy(param->type, "int64_t");
    snprintf(param->cmp_fn, 80, "cmp_write_integer(&cmp, %s)", param->name);
  } else if (strcmp(type, "Boolean") == 0) {
    strcpy(param->type, "bool");
    snprintf(param->cmp_fn, 80, "cmp_write_bool(&cmp, %s)", param->name);
  } else if (strcmp(type, "Array") == 0) {
    strcpy(param->type, "void");
    param->is_arr = true;
  } else if (strncmp(type, "ArrayOf", 7) == 0) {
    // e.g. ArrayOf(Integer, 2)
    char sub_type[20];
    char size[20];
    strncpy(sub_type, type + 8, strlen(type) - 9);
    sub_type[strlen(type) - 9] = '\0';
    char *c = strchr(sub_type, ',');
    if (c == NULL) {
      strcpy(param->type, sub_type);
      sprintf(size, "%s_size", param->name);
    } else {
      // TODO: add struct field to disable implicit size param
      strncpy(param->type, sub_type, c - sub_type);
      param->type[c - sub_type] = '\0';
      strcpy(size, c + 2);
    }
    param->is_arr = true;
    snprintf(param->cmp_fn, 80, "cmp_write_array(&cmp, %s)", size);

    // Translate the type of the elements
    // The cmp_fn needs to access the element, hence the name copy.
    char name[20];
    strcpy(name, param->name);
    snprintf(param->name, 20, "%s[i]", name);

    translate_param(param);

    strcpy(param->name, name);
  } else if (strcmp(type, "Buffer") == 0 || strcmp(type, "Tabpage") == 0 || strcmp(type, "Window") == 0) {
    snprintf(param->cmp_fn, 80, "cmp_write_ext(&cmp, NVIM_EXT_%s, sizeof(%s), &%s)", type, param->name, param->name);
  } else {
    if (strcmp(type, "Object") == 0 || strcmp(type, "Dictionary") == 0) {
      strcpy(param->type, "void");
      param->is_ptr = true;
    }
    snprintf(param->cmp_fn, 80, "cmp_write_str(&cmp, %s, sizeof(%s))", param->name, param->name);
  }
}

void read_parameters (cmp_ctx_t *cmp, param_t **params, uint32_t *num_params) {
  uint32_t not_used, name_size, type_size;
  if (!cmp_read_array(cmp, num_params)) {
    error_and_exit(cmp_strerror(cmp));
  }
  *params = malloc(*num_params * sizeof(param_t));
  for (int i = 0; i < *num_params; i++) {
    name_size = sizeof((*params)[i].name);
    type_size = sizeof((*params)[i].type);
    if (!cmp_read_array(cmp, &not_used)) {
      error_and_exit(cmp_strerror(cmp));
    }
    if (!cmp_read_str(cmp, (*params)[i].type, &name_size)) {
      error_and_exit(cmp_strerror(cmp));
    }
    if (!cmp_read_str(cmp, (*params)[i].name, &type_size)) {
      error_and_exit(cmp_strerror(cmp));
    }
    (*params)[i].is_arr = false;
    (*params)[i].is_out = false;
    (*params)[i].is_ptr = false;
    translate_param(&(*params)[i]);
  }
}

void read_function (cmp_ctx_t *cmp, func_t *fn) {
  uint32_t map_size;
  cmp_read_map(cmp, &map_size);

  fn->deprecated = false;
  uint32_t name_size = sizeof(fn->name);
  uint32_t ret_type_size = sizeof(fn->ret.type);

  for (int i = 0; i < map_size; i++) {
    uint32_t key_size = 20;
    char key[key_size];
    if(!cmp_read_str(cmp, key, &key_size)) {
      error_and_exit(cmp_strerror(cmp));
    }

    if (strcmp(key, "name") == 0) {
      if (!cmp_read_str(cmp, fn->name, &name_size)) {
        error_and_exit(cmp_strerror(cmp));
      }
    } else if (strcmp(key, "return_type") == 0) {
      if (!cmp_read_str(cmp, fn->ret.type, &ret_type_size)) {
        error_and_exit(cmp_strerror(cmp));
      }
      fn->ret.is_out = true;
      strcpy(fn->ret.name, "result");
      translate_param(&fn->ret);
    } else if (strcmp(key, "parameters") == 0) {
      read_parameters(cmp, &fn->params, &fn->num_params);
    } else {
      if (strcmp(key, "deprecated_since") == 0) {
        fn->deprecated = true;
      }
      cmp_object_t cmp_obj;
      if (!cmp_read_object(cmp, &cmp_obj)) {
        error_and_exit(cmp_strerror(cmp));
      }
    }
  }
}

void read_functions (cmp_ctx_t *cmp, func_t **fns, uint32_t *num_funcs) {
  cmp_read_array(cmp, num_funcs);
  *fns = malloc(*num_funcs * sizeof(func_t));

  for (int i = 0; i < *num_funcs; i++) {
    read_function(cmp, &((*fns)[i]));
  }
}

void read_error_types (cmp_ctx_t *cmp) {
  uint32_t map_size;
  if (!cmp_read_map(cmp, &map_size)) {
    error_and_exit(cmp_strerror(cmp));
  }

  uint32_t key_size = 12;
  char key[key_size];
  uint32_t id_size = 10;
  char id_key[id_size];
  uint8_t id;
  for (int i = 0; i < map_size; i++) {
    if(!cmp_read_str(cmp, key, &key_size)) {
      error_and_exit(cmp_strerror(cmp));
    }
    key_size = 12;

    uint32_t sub_map_size;
    if (!cmp_read_map(cmp, &sub_map_size)) {
      error_and_exit(cmp_strerror(cmp));
    }

    if(!cmp_read_str(cmp, id_key, &id_size)) {
      error_and_exit(cmp_strerror(cmp));
    }
    id_size = 10;

    if (!cmp_read_pfix(cmp, &id)) {
      error_and_exit(cmp_strerror(cmp));
    }
  }
}

void read_types (cmp_ctx_t *cmp) {
  uint32_t map_size;
  if (!cmp_read_map(cmp, &map_size)) {
    error_and_exit(cmp_strerror(cmp));
  }

  uint32_t key_size = 12;
  char key[key_size];
  uint32_t sub_key_size = 20;
  char sub_key[sub_key_size]; 
  uint32_t prefix_size = 14;
  char prefix[prefix_size];

  uint8_t id;
  for (int i = 0; i < map_size; i++) {
    if(!cmp_read_str(cmp, key, &key_size)) {
      error_and_exit(cmp_strerror(cmp));
    }
    key_size = 12;

    uint32_t sub_map_size;
    if (!cmp_read_map(cmp, &sub_map_size)) {
      error_and_exit(cmp_strerror(cmp));
    }

    for (int j = 0; j < sub_map_size; j++) {
      if(!cmp_read_str(cmp, sub_key, &sub_key_size)) {
        error_and_exit(cmp_strerror(cmp));
      }
      sub_key_size = 20;

      if (strcmp(sub_key, "id") == 0) {
        if (!cmp_read_pfix(cmp, &id)) {
          error_and_exit(cmp_strerror(cmp));
        }
      } else {
        if(!cmp_read_str(cmp, prefix, &prefix_size)) {
          error_and_exit(cmp_strerror(cmp));
        }
        prefix_size = 14;
      }
    }

    fprintf(out_h_file, "typedef uint8_t %s;\n", key);
    fprintf(out_h_file, "#define NVIM_EXT_%s %i\n", key, id);
  }
  fprintf(out_c_file, "\n");
}

void read_map (cmp_ctx_t *cmp, cmp_object_t cmp_obj) {
  uint32_t map_size = cmp_obj.as.map_size;
  uint32_t key_size = 12;
  uint32_t num_fns;
  func_t *fns;
  char key[key_size];
  for (int i = 0; i < map_size; i++) {
    if(!cmp_read_str(cmp, key, &key_size)) {
      error_and_exit(cmp_strerror(cmp));
    }
    key_size = 12;

    if (strcmp(key, "version") == 0) {
      read_version(cmp);
    } else if (strcmp(key, "functions") == 0) {
      read_functions(cmp, &fns, &num_fns);
    } else if (strcmp(key, "error_types") == 0) {
      read_error_types(cmp);
    } else if (strcmp(key, "types") == 0) {
      read_types(cmp);
    } else {
      fprintf(stderr, "unknown root key: %s\n", key);
      exit(1);
    }
  }

  for (int i = 0; i < num_fns; i++) {
    if (!fns[i].deprecated && strcmp(fns[i].name, "nvim_call_function") != 0
        && strcmp(fns[i].name, "nvim_call_atomic") != 0 && strcmp(fns[i].name, "nvim_get_api_info") != 0) {
      print_function(&fns[i]);
    }
    free(fns[i].params);
  }
  free(fns);
}

void cp (FILE *file_a, FILE *file_b) {
  size_t num_read, num_written;
  char buf[4096];
  while ((num_read = fread(buf, sizeof(char), 4096, file_a)) > 0) {
    num_written = fwrite(buf, sizeof(char), num_read, file_b);
    if (num_read != num_written) {
      fprintf(stderr, "cp: read %zu, wrote %zu\n", num_read, num_written);
    }
  }
}

int main (void) {
  FILE *api_info_file = popen("nvim --api-info", "r");
  if (api_info_file == NULL) {
    fprintf(stderr, "failed to run nvim command\n");
    exit(1);
  }

  out_h_file = fopen("nvim_rpc.h", "w");
  out_c_file = fopen("nvim_rpc.c", "w");

  FILE *rpc_base_h_file = fopen("scripts/rpc_base.h", "r");
  fprintf(out_h_file, "#ifndef NVIM_RPC_H_\n#define NVIM_RPC_H_\n\n");
  cp(rpc_base_h_file, out_h_file);
  fclose(rpc_base_h_file);

  FILE *rpc_base_c_file = fopen("scripts/rpc_base.c", "r");
  cp(rpc_base_c_file, out_c_file);
  fclose(rpc_base_c_file);

  fprintf(out_c_file, "\n");

  cmp_ctx_t cmp;
  cmp_init(&cmp, api_info_file, file_reader, null_writer);

  cmp_object_t cmp_obj;
  cmp_read_object(&cmp, &cmp_obj);
  if (cmp_obj.type == CMP_TYPE_FIXMAP) {
    read_map(&cmp, cmp_obj);
  } else {
    fprintf(stderr, "unknown type: %i\n", cmp_obj.type);
    exit(1);
  }

  fprintf(out_h_file, "#endif");

  pclose(api_info_file);
  fclose(out_h_file);
  fclose(out_c_file);
  return 0;
}
