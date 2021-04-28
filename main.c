#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_DATA 256
#define MAX_ROWS 100

struct Address {
	int id;
	int set;
	char name[MAX_DATA];
	char email[MAX_DATA];
};

struct Database {
	struct Address rows[MAX_ROWS];
};

struct Connection {
	struct Database *db;
	FILE *file;
};

void die(const char *message) {
	if (errno) {
		perror(message);
	} else {
		printf("error: %s\n", message);
	}

	exit(1);
}

size_t Database_load(struct Connection *conn) {
	size_t res = fread(conn->db, sizeof(struct Database), 1, conn->file);
	return res;	
}

void Database_write(struct Connection *conn) {
	rewind(conn->file);

	int rc = fwrite(conn->db, sizeof(struct Database), 1, conn->file);
	if (rc != 1)
		die("Failed to write database");

	fflush(conn->file);
}

struct Connection *Database_connect(const char *filename, const char action) {
	struct Connection *conn = malloc(sizeof(struct Connection));
	if (!conn)
		die("failed allocating memory for connection");

	conn->db = malloc(sizeof(struct Database));
	if (!conn->db)
		die("failed allocating memory for database");

	if (action == 'c') {
		conn->file = fopen(filename, "w");
	} else {
		conn->file = fopen(filename, "r+");

		if (Database_load(conn) != 1)
			die("error reading from database");
	}

	if (!conn->file)
		die("error opening database");

	return conn;
}

void Database_create(struct Connection *conn) {
	int i = 0;
	for (i = 0; i < MAX_ROWS; i++) {
		struct Address addr = {.id=i,.set=0};
		conn->db->rows[i] = addr;
	}
}

void Database_set(struct Connection *conn, 
		const int id, const char *name, const char *email)
{
	struct Address *addr = &conn->db->rows[id];

	if (addr->set)
		die("There is already record with such id");

	addr->set = 1;

	if (!strncpy(addr->name, name, MAX_DATA))
		die("error setting name");
	addr->name[MAX_DATA-1] = '\0';

	if (!strncpy(addr->email, email, MAX_DATA))
		die("error setting email");
	addr->email[MAX_DATA-1] = '\0';
}

void Address_print(struct Address *addr) {
	printf("%d %s %s\n", addr->id, addr->name, addr->email);
}

void Database_get(struct Connection *conn, const int id) {
	struct Address *addr = &conn->db->rows[id];
	if (addr->set == 0)
		die("There is no record with such id");

	Address_print(addr);
}

void Database_list(struct Connection *conn) {
	int i = 0;
	for (i = 0; i < MAX_ROWS; i++) {
		struct Address *addr = &conn->db->rows[i];
		if (addr->set == 1)
			Address_print(addr);
	}
}

void Database_modify(struct Connection *conn, const int id)
{
	struct Address *addr = &conn->db->rows[id];

	if (id < 0 || id > MAX_ROWS-1)
		die("There is not so many records");
	
	int inp;
	Address_print(addr);
	printf("What to change?\n1.name\n2.email\n3.name and email\n\n> ");
	scanf("%d", &inp);

	char new_name[MAX_DATA];
	char new_email[MAX_DATA];

	if (inp == 1) {
		printf("Enter new name: ");
		scanf("%s", new_name);

		if (!strncpy(addr->name, new_name, MAX_DATA))
				die("Error changing name");
		addr->name[MAX_DATA-1] = '\0';
	} else if (inp == 2) {
		printf("Enter new email: ");
		scanf("%s", new_email);

		if (!strncpy(addr->email, new_email, MAX_DATA))
				die("Error changing email");
		addr->email[MAX_DATA-1] = '\0';
	} else if (inp == 3) {
		printf("Enter new name and email: ");
		scanf("%s %s", new_name, new_email);
		
		if (!strncpy(addr->name, new_name, MAX_DATA))
				die("Error changing name");
		addr->name[MAX_DATA-1] = '\0';
		if (!strncpy(addr->email, new_email, MAX_DATA))
				die("Error changing email");
		addr->email[MAX_DATA-1] = '\0';

	} else {
		die("Nothing changed");
	}
}

void Database_delete(struct Connection *conn, const int id) {
	struct Address *addr = &conn->db->rows[id];

	if (addr->set == 0)
		die("There is no record with such id");

	struct Address empt = {.id=id,.set=0};
	*addr = empt;
}

void Database_close(struct Connection *conn) {
	if (conn) {
		if (conn->db) {
			free(conn->db);
		}
		if (conn->file) {
			fclose(conn->file);
		}
		free(conn);
	}
}

int main(int argc, char *argv[])
{
	// ./main <filename> <action> [parameters]
	if (argc < 3)
	   die("USAGE: ./main <filename> <action> [parameters]");	

	const char *filename = argv[1];
	const char action = argv[2][0];
	int id = 0;
	if (argc > 3)
		id = atoi(argv[3]);

	if (id < 0 || id >= MAX_DATA)
		die("There is not so many records (0-99)");

	struct Connection *conn = Database_connect(filename, action);

	switch (action) {
		case 'c':
			if (argc != 3)
				die("creating: ./main <filename> c");
			Database_create(conn);
			Database_write(conn);
			break;
			
		case 's':
			if (argc != 6)
				die("settting: ./main <filename> s <id> <name> <email>");
			Database_set(conn, atoi(argv[3]), argv[4], argv[5]);
			Database_write(conn);
			break;

		case 'g':
			if (argc != 4)
				die("getting: ./main <filename> g <id>");
			Database_get(conn, id);
			break;

		case 'd':
			if (argc != 4)
				die("deleting: ./main <filename> d <id>");
			Database_delete(conn, id);
			Database_write(conn);
			break;

		case 'm':
			if (argc != 4)
				die("modifying: ./main <filename> m <id>");
			Database_modify(conn, id);	
			Database_write(conn);
			break;

		case 'l':
			if (argc != 3)
				die("listing: ./main <filename> l");
			Database_list(conn);
			break;

		default:
			die("Unknown command.\nc-create\ns-set\ng-get\nd-delete\nm-modify\nl-list\n");
	}

	Database_close(conn);

	return 0;
}

