#include "sh_list.h"
#include "sh_lib.h"

struct data_example {
    sh_list_t node;
    int value;
};

struct data_example data;

int main(int argc, char **argv)
{
    /* 定义、初始化链表头节点 */
    SH_LIST_INIT(head);

    /* 手动初始化节点 */
    sh_list_init(&data.node);
    data.value = 1;

    /* 链表节点插入 */
    sh_list_insert_after(&data.node, &head);

    /* 链表遍历 */
    sh_list_for_each(node, &head) {
        /* 获取插入链表中的对象地址 */
        struct data_example *data_temp = sh_container_of(node, struct data_example, node);
        printf("data.value = %d\r\n", data_temp->value);
    }

    /* 链表节点删除 */
    sh_list_remove(&data.node);
}




