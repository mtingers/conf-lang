#include <my_global.h>
#include <mysql.h>
#include <string.h>

void die(void *p, ...)
{

}

static MYSQL *db_connect(const char *user, const char *pass, const char *host)
{
    MYSQL *conn = NULL;
    conn = mysql_init(NULL);
    if(!conn)
        die(NULL, "ERROR: %u: %s", mysql_errno(conn), mysql_error(conn));

    if(!mysql_real_connect(conn, host, user, pass, NULL, 0, NULL, 0))
        die(NULL, "ERROR: %u: %s\n", mysql_errno(conn), mysql_error(conn));

    return conn;
}

static MYSQL_RES *db_query(MYSQL *conn, const char *query)
{
    //my_ulonglong nrows;
    MYSQL_RES *result;

    mysql_real_query(conn, query, strlen(query));
    result = mysql_store_result(conn);
    /* questionable
    nrows = mysql_num_fields(result);
    if(nrows < 1) {
        mysql_free_result(result);
        return NULL;
    }*/
    return result;
}

static unsigned long db_escape_string(MYSQL *conn, const char *string, char *dst, unsigned long string_len)
{
    return mysql_real_escape_string(conn, dst, string, string_len);
}

static my_ulonglong db_num_fields(MYSQL_RES *result)
{
    return mysql_num_fields(result);
}

static my_ulonglong db_num_rows(MYSQL_RES *result)
{
    return mysql_num_rows(result);
}

struct db_connector {
    MYSQL           *(*db_connect)(const char *user, const char *pass, const char *host);
    MYSQL_RES       *(*db_query)(MYSQL *conn, const char *query);
    unsigned long   (*db_escape_string)(MYSQL *conn, const char *string, char *dst, unsigned long string_len);
    void            *(*db_disconnect)(const char *user, const char *pass, const char *host);
    my_ulonglong    (*db_num_rows)(MYSQL_RES *result);
    my_ulonglong    (*db_num_fields)(MYSQL_RES *result);
};

static struct db_connector mysql_db_connector = {
    db_connect,
    db_query,
    db_escape_string,
    NULL,
    db_num_rows,
    db_num_fields,
};


/* Public API */
struct db_connector *db_get_mysql_connector()
{
    struct db_connector *p = &mysql_db_connector;
    return p;
}

int main(void)
{
    return 0;
}
