/*
 4: Student Name: Siqi Cao
 5: Terp Connect login ID: scao
 6: UID Number: 114545657
 7: Section number: 0301
 8:
 9: I pledge on my honor that I have not given or received
 10: any unauthorized assistance on this assignment.
 */

#include <stdio.h>
#include "string.h"
#include "stdlib.h"
#include "fs-sim-datastructure.h"

void mkfs(Fs_sim *files)
{
    /* allocates memory for root */
    Node *node = malloc(sizeof(Node));

    /* initializes root node */
    node->name = "/";
    node->child = NULL;
    node->next = NULL;
    node->parent = NULL;
    node->type = TYPE_DIR;

    /* initializes file system */
    files->root = node;
    files->current = node;
}

int touch(Fs_sim *files, const char arg[])
{
    /*temp points to the first element of the current directory*/
    Node *temp = files->current->child;
    /*when insert a file into the file system,
     use less_than_arg to locate where
     the new file should be inserted in the increasing order.
     less_than_arg is the previous element of the new file.*/
    /*create a new file and assign its fields*/
    Node *new_file = malloc(sizeof(Node));
    new_file->name = malloc(strlen(arg) + 1);
    new_file->name = strcpy(new_file->name, arg);
    new_file->type = TYPE_FILE;
    new_file->parent = files->current;
    /*if arg is an empty string*/
    if (strcmp(arg, "") == 0)
    {
        return 0;
    }
    /*if arg contains "/" somewhere*/
    if (strchr((char*) arg, '/') != NULL)
    {
        return 0;
    }
    /*if arg is ".", "..", or "/"*/
    if (strcmp(arg, ".") == 0 || strcmp(arg, "..") == 0
            || strcmp(arg, "/") == 0)
    {
        return 0;
    }
    /*if the current directory is empty*/
    if (temp == NULL || strcmp(temp->name, new_file->name) > 0)
    {
        /*creat a file in the empty current directory*/
        new_file->parent = files->current;
        new_file->child = NULL;
        new_file->next = temp;
        files->current->child = new_file;
        return 1;
    } else
    {
        /* locate the position to insert */
        while (temp->next && strcmp(temp->next->name, arg) < 0)
        {
            /* duplicate */
            if (strcmp(temp->name, arg) == 0)
            {
                return 0;
            }
            temp = temp->next;
        }

        /*insert a new file in the increasing order*/
        new_file->next = temp->next;
        temp->next = new_file;

        return 1;

    }

    return 0;
}

int mkdir(Fs_sim *files, const char arg[])
{
    /*temp points to the first element of the current directory*/
    Node *temp = files->current->child;

    /*when insert a file into the file system,
     use less_than_arg to locate where
     the new file should be inserted in the increasing order.
     less_than_arg is the previous element of the new file.*/
    /*create a new file and assign its fields*/
    Node *new_file = malloc(sizeof(Node));
    new_file->name = malloc(strlen(arg) + 1);
    new_file->name = strcpy(new_file->name, arg);
    new_file->type = TYPE_DIR;
    new_file->child = NULL;
    new_file->parent = files->current;

    /*if arg is an empty string*/
    if (strcmp(arg, "") == 0)
    {
        return 0;
    }

    /*if arg contains "/" somewhere*/
    if (strchr((char*) arg, '/') != NULL)
    {
        return 0;
    }

    /*if arg is ".", "..", or "/"*/
    if (strcmp(arg, ".") == 0 || strcmp(arg, "..") == 0
            || strcmp(arg, "/") == 0)
    {
        return 0;
    }

    /*if the current directory is empty*/
    if (temp == NULL || strcmp(temp->name, new_file->name) > 0)
    {
        /*creat a file in the empty current directory*/
        new_file->parent = files->current;
        new_file->child = NULL;
        new_file->next = temp;
        files->current->child = new_file;
        return 1;
        /*if the current directory is not empty*/
    } else
    {
        while (temp->next && strcmp(temp->next->name, arg) < 0)
        {
            /* duplicate */
            if (strcmp(temp->name, arg) == 0)
            {
                return 0;
            }
            temp = temp->next;
        }
        /*insert a new file in the increasing order*/
        new_file->next = temp->next;
        temp->next = new_file;
        return 1;
    }

    return 0;
}

int cd(Fs_sim *files, const char arg[])
{
    /* nothing to do with "." */
    if (strcmp(arg, ".") == 0)
    {
        return 0;
    }

    /*if arg is ".."*/
    if (strcmp(arg, "..") == 0)
    {
        /*if current directory is the root directory*/
        if (files->current->parent == NULL)
        {
            return 0;
        }
        /*if current directory is not the root directory*/
        else
        {
            /*parent of the current directory becomes current*/
            files->current = files->current->parent;
            return 1;
        }
    }

    if (strcmp(arg, "/") == 0 || strcmp(arg, "") == 0)
    {
        /*set root to be the current directory*/
        files->current = files->root;
        return 1;
    }

    /*if arg has "/" somewhere*/
    if (strchr((char*) arg, '/') != NULL)
    {
        return 0;
    }

    /*temp points to the first element of the current directory*/
    Node *temp = files->current->child;

    /* searches by name */
    while (temp && strcmp(temp->name, arg) != 0)
    {
        temp = temp->next;
    }

    /*if arg does not refer to an exsiting file or directory
     located in the current directory, and is not special
     values*/
    if (temp == NULL)
    {
        return 0;
    } else
    {
        /*if arg is a file exists in the current directory*/
        if (temp->type == TYPE_FILE)
        {
            return 0;
        }

        /*if arg is a directory exists in the current directory*/
        if (temp->type == TYPE_DIR)
        {
            files->current = temp; /*change the current directory to temp*/
            return 1;
        }
    }

    return 0;
}

int ls(Fs_sim *files, const char arg[])
{
    /*temp points to the first element of the current directory*/
    Node *temp = files->current->child;

    /*sub_temp is the child node of temp when temp is not null*/
    Node *sub_temp;

    if (strcmp(arg, ".") == 0 || strcmp(arg, "") == 0)
    {
        temp = files->current;
    } else if (strcmp(arg, "..") == 0)
    {
        temp = files->current->parent;
    } else if (strcmp(arg, "/") == 0)
    {
        temp = files->root;
    } else if (strchr((char*) arg, '/') != NULL)
    {
        return 0;
    } else
    {
        /* finds by name */
        while (temp && strcmp(temp->name, arg) != 0)
        {
            temp = temp->next;
        }
    }

    /* no such file or dir */
    if (temp == NULL)
    {
        return 0;
    }

    /*if arg is a file exists in the current directory*/
    if (temp->type == TYPE_FILE)
    {
        printf("%s\n", temp->name);
    }
    /*if arg is a directory exists in the current directory*/
    else if (temp->type == TYPE_DIR)
    {
        sub_temp = temp->child;
        /*if the subdirectory has no contents*/
        if (sub_temp == NULL)
        {
            return 0;
        }
        /*if the subdirectory is not null*/
        else
        {
            while (sub_temp != NULL)
            {
                if (sub_temp->type == 0) /*if meets a file*/
                    printf("%s\n", sub_temp->name);
                if (sub_temp->type == 1) /*if meets a directory*/
                    printf("%s/\n", sub_temp->name);
                sub_temp = sub_temp->next;
            }
            return 1;
        }
    }

    return 0;
}

/* recursive function to print pathname */
void path(Node* node)
{
    if (node->parent != NULL)
    {
        /* call itself recursively */
        path(node->parent);

        printf("/");
        printf(node->name);
    }
}

void pwd(Fs_sim *files)
{
    /* root */
    if (files->current->parent == NULL)
    {
        printf("/\n");
    } else
    {
        /* call recursive function to print pathname */
        path(files->current);

        printf("\n");
    }
}

void free_node(Node * n)
{

    /*temp1, temp2 is the pointer to a node, use to traverse the linked list */
    Node * temp1, temp2;
    temp1 = n->child;

    /* free the child of the node*/
    while (temp1 != NULL)
    {
        temp2 = temp1;

        /*move the temp1 to next node*/
        temp1 = temp1->next;

        if (temp2.child != NULL)
        {
            /* call itself recursively */
            free_node(temp2);
        } else
        {
            free(temp2.name);
            free(temp2);
        }
    }

    /*at last free the node itself*/
    free(n->name);
    free(n);
}

void rmfs(Fs_sim *files)
{

    free_node(files->root);
}

int rm(Fs_sim *files, const char arg[])
{
    /*temp points to the first element of the current directory*/
    Node *temp1 = files->current->child;
    Node *temp2 = NULL;

    if (strcmp(arg, ".")
            == 0|| strcmp(arg, "") == 0 || (strcmp(arg, "..") == 0) || strchr((char*) arg, '/') != NULL)
    {
        return 0;
    } else
    {
        /* finds by name */
        while (temp1 && strcmp(temp1->name, arg) != 0)
        {
            temp2 = temp1;
            temp1 = temp1->next;
        }
    }

    /* no such file or dir */
    if (temp1 == NULL)
    {
        return 0;
    }

    /*do not need to consider the FILE or DIR type, the free_node method
     * will implement different stratgys*/
    if (temp2 == NULL)
    {
        /* is the only first child of the current node */
        free_node(temp1);
        files->current->child = NULL;
    } else
    {
        temp2->next = temp1->next;
        free_node(temp1);
    }

    return 0;
}
