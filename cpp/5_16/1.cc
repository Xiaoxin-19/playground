#include <iostream>
#include <algorithm>
#include <cmath>
using namespace std;


int n;
int nums[10005];
int idx[10005];



struct Node
{
    Node *pre = nullptr;
    Node *next = nullptr;
    int val = -1;
    int idx = -1;
    Node(int v, int i):val(v),idx(i){}
};

struct List
{
    Node * head = nullptr;
    Node * tail = nullptr;
};

List list;

Node* insert_at_tail(List * list, int val_idx)
{
    Node *new_node = new Node(nums[val_idx],val_idx);
    if(list->head == nullptr)
    {
        list->tail = list->head = new_node;
        return new_node;
    }
    
    // 连接之后的链表
    new_node->next = list->tail->next;
    // 连接之前的链表
    new_node->pre = list->tail;
    
    // 链表连接新结点
    list->tail->next = new_node;
    
    // 更新尾部结点
    list->tail = new_node;
    return new_node;
}


void remove_node(Node * node)
{
    if(node == nullptr) return;

    cout<<"the node value: "<<node->val<<endl;
    if(node->pre != nullptr)
    {
          node->pre->next = node->next;
    }
    if(node->next != nullptr)
    {
        node->next->pre = node->pre;
    }
    delete node;
    node = nullptr;
}

void print_list()
{
    Node *p = list.head;
    while(p != nullptr)
    {
        cout<<p->val<<" ";
        p = p->next;
    }
    cout<<endl;
    return ;
}

Node *ass[10005];

void print_arr()
{
    for(int i = 1 ; i <= n ;)
    {
        cout<<nums[idx[i]]<<" ";
        ++i;
    }
    cout<<endl;
}

int ans[10005];

int main()
{
    cin >> n; 
    for(int i = 1 ; i <= n ; i++)
    {
        scanf("%d", &nums[i]);   
        idx[i] = i;
    }
    
    // 1. 将输入数组排序，此时数组有序
    sort(idx+1, idx+n+1, [&](int idx_i, int idx_j){return nums[idx_i] < nums[idx_j];});
    // print_arr();
    
    // 2. 将排序后的数组生成一个链表
    for(int i = 1; i <= n ; i++)
    {
        ass[idx[i]] = insert_at_tail(&list, idx[i]);
    }
     
    print_list();
    // 3. 处理从未排序的数组: 从后到前处理： 简化了操作， 从后往前处理就可以保证 前驱 j 一定时 小于 i的
    // 4. 为了实现第3步需要建立数组和链表对应结点的关联
    for(int i = n; i > 1 ; i--)
    {
        long long curr_val = nums[i];
        // cout<<"curr_val: "<< curr_val<<endl;
        Node* curr_node = ass[i];
        cout<<"curr_node: "<<curr_node->val<<endl;
        
        long long pre_val = nums[idx[1]] - 1e9;
        if(curr_node->pre != nullptr)
        {
            pre_val = curr_node->pre->val;
        }
        long long  next_val = nums[idx[n]] + 4e9;
        if(curr_node->next != nullptr)
        {
            next_val = curr_node->next->val;
        }
        ans[i] = curr_val - pre_val <= next_val - curr_val ? curr_node->pre->idx : curr_node->next->idx;
        cout<<"target_val: "<<nums[ans[i]]<<endl;
        // cout<<"-----------"<<endl;
        remove_node(curr_node);
    }
    
    for(int i = 2 ; i <= n; i++)
    {
        long long curr_val = nums[i];
        // cout<<"curr_val: "<<curr_val<<" target_val: "<<nums[ans[i]]<<endl;
        printf("%d %d\n", abs(curr_val - nums[ans[i]]), ans[i]);
    }
    return 0;
}