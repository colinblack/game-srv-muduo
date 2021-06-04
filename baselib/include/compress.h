#ifndef COMPRESS_H_
#define COMPRESS_H_

/*
 * 针对可打印字符的huffman解码.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

/* 描述huffman树的节点 */
typedef struct huffman_node_tag
{
    unsigned char        isLeaf;       // 当前节点是否为叶子节点
    unsigned long        count;        // 权(字符出现的次数)
    struct huffman_node_tag *parent;   // 当前节点的父亲
    union 
    {
        struct 
        {
            struct huffman_node_tag *zero;  // 左孩子
            struct huffman_node_tag *one;   // 右孩子
        };
        unsigned char symbol;    // 存储的字符.
    };
} huffman_node;

class CCompress
{  
    /**
     * bit-->byte转换, 不足8位则补齐8位.
     */
    static unsigned long numbytes_from_numbits(unsigned long numbits);

    /*
     * 取出第i位(从低位往高位排)
     */
    static unsigned char get_bit(unsigned char *bits, unsigned long i);

    /**
     * 创建一个孤立的"叶子"节点, 该节点为一个单独的树
     * symbol : 该叶子节点的权,即字符
     */
    static huffman_node* new_leaf_node(unsigned char symbol);

    /**
     * 创建节点(该节点不是叶子节点)
     * count  : 字符出现的次数
     * zero   : 左孩子
     * one    : 右兄弟
     * return : 返回一个节点对象
     */
    static huffman_node* new_nonleaf_node(unsigned long count, huffman_node *zero, huffman_node *one);

    /**
     * 释放树占用的内存空间
     */
    static void free_huffman_tree(huffman_node *subtree);

    /*
     * 创建树
     */
    static huffman_node* read_code_table();

public:
    /*
    * 获取真实压缩码
    * <= 0 成功 返回码长
    * -1   失败 非法字码
    */
    static int conv256(const char * inBytes,int inlength,char * outBytes,int outlength);
    static int conv64(const char * inBytes,int inlength,char * outBytes,int outlength);


    /**
     * 对FILE进行解码
     * 0  成功
     * -1 失败，码长<2
     */
    static int huffman_decode(const unsigned char *encode,int encodesize, unsigned char *result,int resultlen);

    static int zlib_compress(const char * inBytes,int inlength,char * outBytes,int outlength);
    static int zlib_uncompress(const char * inBytes,int inlength,char * outBytes,int outlength);
};
 
#endif
