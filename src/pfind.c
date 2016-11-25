#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>
#include <linux/limits.h>
#include <string.h>
#include <fnmatch.h>
#include <errno.h>
#include <omp.h>

char search_path[PATH_MAX];
char filename[NAME_MAX];
rlim_t nofile_per_thread;

typedef struct vector_t {
	char **_str;
	size_t _size;
} vector_t;

static vector_t *vec_push(vector_t *q, const char *str)
{
	vector_t *p = NULL;
	static size_t size = 1;

	if (!q) {
		p = (vector_t*)malloc(sizeof(*p));
		p->_str = (char**)malloc(sizeof(*(p->_str))*size);
		p->_str[size-1] = malloc(sizeof(*str)*(strlen(str)+1));
		strcpy(p->_str[size-1], str);
		p->_size = size++;
		return p;
	} else {
		q->_str = (char**)realloc(q->_str, sizeof(*(q->_str))*size);
		q->_str[size-1] = malloc(sizeof(*str)*(strlen(str)+1));
		strcpy(q->_str[size-1], str);
		q->_size = size++;
		return q;
	}

}

#if 0
static char *vec_pop(vector_t *p)
{
	char *str = NULL;
	size_t size = p->_size;
	
	if (!size)
		return NULL;
	
	str = strdup(p->_str[size-1]);
	free(p->_str[size-1]);
	p->_size--;
	p->_str = (char**)realloc(p->_str, sizeof(*(p->_str))*size);
	return str;
}
#endif

static void vec_clear(vector_t *p)
{
	int i;
	size_t size = p->_size;
	for (i=0; i<size; i++) {
		free(p->_str[i]);
	}
	free(p->_str);
	free(p);
}

static void get_nofile(void)
{
	struct rlimit rlim;
	
	if ((getrlimit(RLIMIT_NOFILE, &rlim))) {
		perror("getrlimit");
		exit(EXIT_FAILURE);
	}
	#pragma omp parallel
	{
		#pragma omp single nowait
		{
			
			nofile_per_thread =rlim.rlim_cur / omp_get_num_threads();
		}
	}
}

static void parse_args(int argc, char **argv)
{
	char *p;
	
	if (argc != 3) {
		fprintf(stderr, "usage: %s <dir> <file name>\n", argv[0]);
		fprintf(stderr, "currently only a file can be searched.\n");
		exit(EXIT_FAILURE);
	}
	memset(search_path, '\0', sizeof(search_path));
	memset(filename,    '\0', sizeof(filename));
	strcpy(search_path, argv[1]);
	strcpy(filename,    argv[2]);
	if ((p = strrchr(search_path, '/'))) {
		*p = '\0';
	}
	get_nofile();
}

static DIR* check_dir(void)
{
	DIR *dir;
	
	if (!(dir = opendir(search_path))) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}
	return dir;
}

/* recursive search */
static void rsearch(const char *path)
{
	struct dirent *dp;
	DIR *dir;
	int fd;
	
       	if (!((dir = opendir(path)))) {
		fprintf(stderr, "%s: %s\n",path, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (-1 == (fd = dirfd(dir))) {
			fprintf(stderr, "error in dirfd()\n");
			exit(EXIT_FAILURE);
	}
	if (fd == nofile_per_thread) { /* need to add a good handling for nofile per thread */
		fprintf(stderr, "reached nofile limit\n");
		exit(EXIT_FAILURE);
	}
	
	while ((dp = readdir(dir))) {
		/* due to a bug of NFS, sometimes DT_UNKNOWN is returned... */
		if ((dp->d_type == DT_REG || dp->d_type == DT_UNKNOWN) && !fnmatch(filename, dp->d_name, FNM_PATHNAME)) {
			printf("%s/%s\n", path, dp->d_name);
		} else if (dp->d_type == DT_DIR && strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
			char buf[PATH_MAX];
			memset(buf, '\0', sizeof(buf));
			strcat(buf, path);
			strcat(buf, "/");
			strcat(buf, dp->d_name);
			rsearch(buf);
		}
	}
	if (closedir(dir)) {
		perror("closedir");
		exit(EXIT_FAILURE);
	}
}

static void pfind(vector_t *vp)
{
	int i;
#pragma omp parallel for private(i)
	for (i=0; i<vp->_size; i++) 
		rsearch(vp->_str[i]);
}

/* detph 1, get all of dirnames to parallelize */
static void readd(DIR *dir)
{
	struct dirent *dp;
	vector_t *vp = NULL;
	
	while((dp = readdir(dir))) {
		/* due to a bug of NFS, sometimes DT_UNKNOWN is returned... */
		if (dp->d_type == DT_REG || dp->d_type == DT_UNKNOWN) {
			if (!fnmatch(filename, dp->d_name, FNM_PATHNAME)) {
				printf("%s/%s\n", search_path, dp->d_name);
			}
		} else if (dp->d_type == DT_DIR && strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
			char buf[NAME_MAX];
			memset(buf, '\0', sizeof(buf));
			strcat(buf, search_path);
			strcat(buf, "/");
			strcat(buf, dp->d_name);
			vp = vec_push(vp, buf);
		}
	}
	pfind(vp);
	vec_clear(vp);
	closedir(dir);
}

int main(int argc, char **argv)
{
	DIR *dir;
	
	parse_args(argc, argv);
	dir = check_dir();
	readd(dir);
	return 0;
}
