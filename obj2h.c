#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef float float3[3];
typedef int	  int3[3];

enum { X, Y, Z };

void obj2h(const char* p) {
	FILE* fh = fopen(p, "r");

	size_t p_len = strlen(p);
	char p_new[256];
	memcpy(p_new, p, p_len - 4);
	strcat(p_new, "_trimesh_obj.h");

	FILE* out_fh = fopen(p_new, "w");

	if (!fh) {
		fprintf(stderr, "load_obj failed to open\"%s\"\n", p);
		exit(-1);
	}

	if (!out_fh) {
		fprintf(stderr, "load_obj failed to create output file \"%s\"\n", p_new);
		exit(-1);
	}

	char buff[128];
	char obj_name[128];
	int num_v = 0,
			num_f  = 0;
	while (1) {
		fgets(buff, sizeof(buff), fh);
		if (feof(fh))
			break;

		if (buff[0] == '#' || buff[0] == '\n')
			continue;

		switch (buff[0]) {
			case 'o':
				sscanf(buff, "o %s", obj_name);
				break;
			case 'v':
				switch (buff[1]) {
					case ' ':
						num_v += 1;
						break;
					default:
						break;
				}
				break;
			case 'f':
				num_f		+= 1;
				break;
			default:
				break;
		}
	}

	float3* v_indices = (float3*)malloc(num_v * sizeof(float3));
	int*    f_indices = (int*)malloc(num_f * 3 * sizeof(int));
	int v_index = 0,
			f_index  = 0;

	rewind(fh);
	while (1) {
		fgets(buff, sizeof(buff), fh);
		if (feof(fh))
			break;

		if (buff[0] == '#' || buff[0] == '\n')
			continue;

		switch (buff[0]) {
			case 'v':
				switch (buff[1]) {
					case ' ':
						sscanf(buff, "v %f %f %f",
								&v_indices[v_index][X],
								&v_indices[v_index][Y],
								&v_indices[v_index][Z]);
						v_index++;
						break;
					default:
						break;
				}
				break;
			case 'f':
				if (sscanf(buff, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d",
							&f_indices[f_index], &f_indices[f_index + 1], &f_indices[f_index + 2]) == 3) {
				} else if (sscanf(buff, "f %d//%*d %d//%*d %d//%*d",
							&f_indices[f_index], &f_indices[f_index + 1], &f_indices[f_index + 2]) == 3) {
				} else if (sscanf(buff, "f %d/%*d %d/%*d %d/%*d",
							&f_indices[f_index], &f_indices[f_index + 1], &f_indices[f_index + 2]) == 3) {
				} else if (sscanf(buff, "f %d %d %d",
							&f_indices[f_index], &f_indices[f_index + 1], &f_indices[f_index + 2]) == 3) {
				} else {
					fprintf(stderr, "load_obj failed \"%s\" wtf is this \"%s\"?", p, buff);
					exit(-1);
				}

				f_index += 3;
				break;
			default:
				break;
		}
	}

	fprintf(out_fh, "#ifndef __OBJ_%s__H__\n", obj_name);
	fprintf(out_fh, "#define __OBJ_%s__H__\n\n", obj_name);

	fprintf(out_fh, "static float %s_vertices[] = {\n", obj_name);

	for (int i = 0; i < num_v; ++i)
		fprintf(out_fh, "\t%f, %f, %f,\n", v_indices[i][X], v_indices[i][Y], v_indices[i][Z]);

	fprintf(out_fh, "};\n\n");

	fprintf(out_fh, "static uint32_t %s_indices[] = {\n", obj_name);

	for (int i = 0; i < num_f * 3;) {
		fprintf(out_fh, "\t%d, %d, %d,\n", f_indices[i], f_indices[i + 1], f_indices[i + 2]);
		i += 3;
	}

	fprintf(out_fh, "};\n\n");

	fprintf(out_fh, "static const size_t %s_num_vertices  = %d;\n",   obj_name, num_v);
	fprintf(out_fh, "static const size_t %s_num_indices   = %d;\n\n", obj_name, num_f * 3);

	fprintf(out_fh, "#endif // __OBJ_%s__H__\n", obj_name);

	free(v_indices);
	free(f_indices);

	fclose(fh);
	fclose(out_fh);
}

int main(int argc, const char* argv[]) {
	for (int i = 1; i < argc; ++i)
		if (!strcmp(strrchr(argv[i], '.'), ".obj"))
			obj2h(argv[i]);
}
