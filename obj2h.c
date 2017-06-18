#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef float float3[3];
typedef int		int3[3];
typedef float float2[2];

enum { X, Y, Z };

typedef struct __obj_final_t {
	float3 *vertices, *normals;
	float2 *uvs;
} obj_data_t;

void obj2h(const char* p) {
	FILE* fh = fopen(p, "r");
	
	size_t p_len = strlen(p);
	char p_new[256];
	memcpy(p_new, p, p_len - 4);
	strcat(p_new, "_obj.h");
	
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
	num_vt = 0,
	num_vn = 0,
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
					case 'n':
						num_vn += 1;
						break;
					case 't':
						num_vt += 1;
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
	
	obj_data_t data;
	data.vertices = (float3*)malloc(num_v  * sizeof(float3));
	data.normals  = (float3*)malloc(num_vn * sizeof(float3));
	data.uvs      = (float2*)malloc(num_vt * sizeof(float2));
	
	int* v_indices  = (int*)malloc(num_f * 3 * sizeof(int));
	int* vn_indices = (int*)malloc(num_f * 3 * sizeof(int));
	int* vt_indices = (int*)malloc(num_f * 3 * sizeof(int));
	
	int3 verts_tmp, norms_tmp, uvs_tmp;
	int v_index = 0,
			vt_index = 0,
			vn_index = 0,
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
									 &data.vertices[v_index][X],
									 &data.vertices[v_index][Y],
									 &data.vertices[v_index][Z]);
						v_index++;
						break;
					case 'n':
						sscanf(buff, "vn %f %f %f",
									 &data.normals[vn_index][X],
									 &data.normals[vn_index][Y],
									 &data.normals[vn_index][Z]);
						vn_index++;
						break;
					case 't':
						sscanf(buff, "vt %f %f",
									 &data.uvs[vt_index][X],
									 &data.uvs[vt_index][Y]);
						vt_index++;
						break;
					default:
						break;
				}
				break;
			case 'f':
				if (sscanf(buff, "f %d/%d/%d %d/%d/%d %d/%d/%d",
									 &verts_tmp[X], &uvs_tmp[X], &norms_tmp[X],
									 &verts_tmp[Y], &uvs_tmp[Y], &norms_tmp[Y],
									 &verts_tmp[Z], &uvs_tmp[Z], &norms_tmp[Z]) == 9) {
				} else if (sscanf(buff, "f %d//%d %d//%d %d//%d",
													&verts_tmp[X], &norms_tmp[X],
													&verts_tmp[Y], &norms_tmp[Y],
													&verts_tmp[Z], &norms_tmp[Z]) == 6) {
					uvs_tmp[X] = 0;
					uvs_tmp[Y] = 0;
					uvs_tmp[Z] = 0;
				} else if (sscanf(buff, "f %d/%d %d/%d %d/%d",
													&verts_tmp[X], &uvs_tmp[X],
													&verts_tmp[Y], &uvs_tmp[Y],
													&verts_tmp[Z], &uvs_tmp[Z]) == 6) {
					norms_tmp[X] = 0;
					norms_tmp[Y] = 0;
					norms_tmp[Z] = 0;
				} else if (sscanf(buff, "f %d %d %d",
													&verts_tmp[X], &verts_tmp[Y], &verts_tmp[Z]) == 3) {
					uvs_tmp[X] = 0; norms_tmp[X] = 0;
					uvs_tmp[Y] = 0; norms_tmp[X] = 0;
					uvs_tmp[Z] = 0; norms_tmp[X] = 0;
				} else {
					fprintf(stderr, "load_obj failed \"%s\" wtf is this \"%s\"?", p, buff);
					exit(-1);
				}
				
				v_indices[f_index]     = verts_tmp[X];
				v_indices[f_index + 1] = verts_tmp[Y];
				v_indices[f_index + 2] = verts_tmp[Z];
				
				vn_indices[f_index]     = norms_tmp[X];
				vn_indices[f_index + 1] = norms_tmp[Y];
				vn_indices[f_index + 2] = norms_tmp[Z];
				
				vt_indices[f_index]     = uvs_tmp[X];
				vt_indices[f_index + 1] = uvs_tmp[Y];
				vt_indices[f_index + 2] = uvs_tmp[Z];
				
				f_index += 3;
				break;
			default:
				break;
		}
	}
	
	fprintf(out_fh, "#ifndef __OBJ_%s__H__\n", obj_name);
	fprintf(out_fh, "#define __OBJ_%s__H__\n\n", obj_name);
	
	fprintf(out_fh, "static float %s_vertices[] = {\n", obj_name);
	
	for (int i = 0; i < num_f * 3; ++i) {
		float3* tmp = &data.vertices[v_indices[i] - 1];
		fprintf(out_fh, "\t%f, %f, %f,\n", (*tmp)[X], (*tmp)[Y], (*tmp)[Z]);
	}
	
	fprintf(out_fh, "};\n\n");
	
	if (vn_indices[0]) {
		fprintf(out_fh, "static float %s_normals[] = {\n", obj_name);
		
		for (int i = 0; i < num_f * 3; ++i) {
			float3* tmp = &data.normals[vn_indices[i] - 1];
			fprintf(out_fh, "\t%f, %f, %f,\n", (*tmp)[X], (*tmp)[Y], (*tmp)[Z]);
		}
		
		fprintf(out_fh, "};\n\n");
	}
	
	if (vt_indices[0]) {
		fprintf(out_fh, "static float %s_texcoords[] = {\n", obj_name);
		
		for (int i = 0; i < num_f * 3; ++i) {
			float2* tmp = &data.uvs[vn_indices[i] - 1];
			fprintf(out_fh, "\t%f, %f,\n", (*tmp)[X], (*tmp)[Y]);
		}
		
		fprintf(out_fh, "};\n\n");
	}
	
	fprintf(out_fh, "static const size_t %s_num_vertices  = %d;\n",   obj_name, num_f * 3);
	fprintf(out_fh, "static const int    %s_has_normals   = %d;\n",   obj_name, vn_indices[0] != 0);
	fprintf(out_fh, "static const int    %s_num_texcoords = %d;\n\n", obj_name, vt_indices[0] != 0);
	
	fprintf(out_fh, "#endif // __OBJ_%s__H__\n", obj_name);
	
	free(data.vertices);
	free(data.normals);
	free(data.uvs);
	free(v_indices);
	free(vn_indices);
	free(vt_indices);
	
	fclose(fh);
	fclose(out_fh);
}

int main(int argc, const char* argv[]) {
	for (int i = 1; i < argc; ++i)
		if (!strcmp(strrchr(argv[i], '.'), ".obj"))
			obj2h(argv[i]);
}
