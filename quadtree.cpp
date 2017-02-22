#include <iostream>
#include <cmath>
#include <queue>
#include <GLUT/glut.h>
using namespace std;

#define MAXDEPTH 3
#define MINSIZE 0.075

struct quadTreeNode {
    float xc, yc;
    float size;
    bool isLeaf;
    int layer;
    float value;
    quadTreeNode * nw, * ne, * sw, * se;
    quadTreeNode* parent;

    quadTreeNode(float x, float y, float a) {
        xc = x;
        yc = y;
        size = a;
        nw = ne = sw = se = NULL;
        parent = NULL;
        isLeaf = true;
        layer = 0;
    }
};

struct circle {
    float xc, yc, r;
    int N;

    circle(float x, float y, float radius) {
        xc = x;
        yc = y;
        r = radius;
        N = 16;  // N shall be adpate when radius changes//antialising
    }

    void circleDraw() {
        // glColor3f(1.0, 1.0, 1.0);
        glPushMatrix();
        glTranslatef(xc, yc, 0.0);
        glBegin(GL_POLYGON);
            for(int i = 0; i < N; i++) {
                float theta = float(i)/float(N)*2.0*M_PI;
                glVertex2f(r*cos(theta), r*sin(theta));
            }
        glEnd();
        glPopMatrix();
    }

    int intersect(quadTreeNode* t) {
        float hx,hy,cx,cy;
        float ux,uy,vx,vy;
        
        hx = hy = 0.5*t->size;
        cx = t->xc;
        cy = t->yc;

        vx = fabs(xc - cx);
        vy = fabs(yc - cy);
        
        ux = max(vx - hx, 0.0f);
        uy = max(vy - hy, 0.0f);
        
        if(ux*ux + uy*uy > r*r)
            return -1; // no intersect
        if((vx+hx)*(vx+hx) + (vy+hy)*(vy+hy) <= r*r) 
            return 1;// contain

        return 0; // intersect
        // return ux*ux + uy*uy <= r*r;
    }
};


bool isContain(quadTreeNode* root, circle* cir){
    float a = 0.5*root->size;
    // cout << root->xc <<" "<< a << ","<<cir->xc << " " << cir->r << endl;
    if(root->xc+a >= cir->xc+cir->r && root->xc-a <= cir->xc-cir->r
        & root->yc+a >= cir->yc+cir->r && root->yc-a <= cir->yc-cir->r) return true;
    // if(root->xc-a > cir->xc+cir->r) return false;
    // if(root->yc+a < cir->yc-cir->r) return false;
    // if(root->yc-a > cir->yc+cir->r) return false;
    return false;
    // return true;
}


quadTreeNode* findEnclosure(quadTreeNode* root, circle* cir) {
    queue<quadTreeNode*> q;
    quadTreeNode* lowest = NULL;
    q.push(root);
    while(!q.empty()) {
        quadTreeNode* tmp = q.front();
        q.pop();
        if(isContain(tmp, cir)) {
            lowest = tmp;
            if(tmp->se) q.push(tmp->se);
            if(tmp->sw) q.push(tmp->sw);
            if(tmp->ne) q.push(tmp->ne);
            if(tmp->nw) q.push(tmp->nw);
        }
    }
    return lowest;
}

quadTreeNode* findCommonAncesstor(quadTreeNode* root, circle* c1, circle* c2) {
    quadTreeNode* node1 = findEnclosure(root,c1);
    quadTreeNode* node2 = findEnclosure(root,c2);
            // cout << "enclosure node1 center (" << node1->xc <<"," <<node1->yc<<")" << node1->layer <<"\n";
            // cout << "enclosure node2 center (" << node2->xc <<"," <<node2->yc<<")" << node2->layer <<"\n";

    if(node1 == NULL || node2 == NULL) return NULL;

    int n1 = node1->layer;
    int n2 = node2->layer;

    if(n2>n1) {
        for(int i=0; i < n2-n1; i++)
            node2 = node2->parent;
    } else {
        for(int i=0; i < n1-n2; i++)
            node1 = node1->parent;
    }

    while(node1 != node2) {
        node1 = node1->parent;
        node2 = node2->parent;
    }
    return node1;
}


static void refineRegion(quadTreeNode* root) {
    // if(!root.isLeaf) return;
    // root.isLeaf = false;
    root->isLeaf = false;
    float wh = root->size*0.5;
    float wq = root->size*0.25;
    root->nw = new quadTreeNode(root->xc-wq, root->yc+wq, wh);
    root->nw->parent = root;
    root->nw->layer = root->layer+1;
    root->ne = new quadTreeNode(root->xc+wq, root->yc+wq, wh);
    root->ne->parent = root;
    root->ne->layer = root->layer+1;
    root->sw = new quadTreeNode(root->xc-wq, root->yc-wq, wh);
    root->sw->parent = root;
    root->sw->layer = root->layer+1;
    root->se = new quadTreeNode(root->xc+wq, root->yc-wq, wh);
    root->se->parent = root;
    root->se->layer = root->layer+1;
}

void quadTree(quadTreeNode* root, int n) {
    if( n == 0 ) return;
    {
        refineRegion(root);
        quadTree(root->nw, n - 1);
        quadTree(root->ne, n - 1);
        quadTree(root->sw, n - 1);
        quadTree(root->se, n - 1);
    }
}

quadTreeNode* findNeighbor(quadTreeNode* root, quadTreeNode* node, char direction) {
    if(node == root) return NULL;
    
    quadTreeNode* mu;

    switch(direction) {
        case 'n':
            if(node == node->parent->sw)
                return node->parent->nw;
            if(node == node->parent->se)
                return node->parent->ne;

            mu = findNeighbor(root, node->parent, direction);
            if(mu == NULL || mu->isLeaf) {
                return mu;
            } else if(node == node->parent->nw) {
                return mu->sw;
            } else if(node == node->parent->ne) {
                return mu->se;
            }
        case 's':
            if(node == node->parent->nw)
                return node->parent->sw;
            if(node == node->parent->ne)
                return node->parent->se;

            mu = findNeighbor(root, node->parent, direction);
            if(mu == NULL || mu->isLeaf) {
                return mu;
            } else if(node == node->parent->sw) {
                return mu->nw;
            } else if(node == node->parent->se) {
                return mu->ne;
            }
        case 'e':
            if(node == node->parent->nw)
                return node->parent->ne;
            if(node == node->parent->sw)
                return node->parent->se;

            mu = findNeighbor(root, node->parent, direction);
            if(mu == NULL || mu->isLeaf) {
                return mu;
            } else if(node == node->parent->ne) {
                return mu->nw;
            } else if(node == node->parent->se) {
                return mu->sw;
            }
        default://case 'w':
            if(node == node->parent->ne) 
                return node->parent->nw;
            if(node == node->parent->se)
                return node->parent->sw;

            mu = findNeighbor(root, node->parent, direction);
            if(mu == NULL || mu->isLeaf) {
                return mu;
            } else if(node == node->parent->nw) {
                return mu->ne;
            } else if(node == node->parent->sw) {
                return mu->se;
            }
    }
}

float neighborValue(quadTreeNode* root, quadTreeNode* node, char direction, int& layer1 = 0) {
    if(node == root) return NULL;
    
    quadTreeNode* node1;

    node1 = findNeighbor(root, node, direction);
    if(!node1->isLeaf)
        return 0.25f*(node1->nw->value + node1->sw->value + node1->ne->value + node1->se->value);
    
    if(node1->layer == node->layer)
        return node1->value;

    // node1->depth = node->depth + 1
    layer1 = node1->layer;
    int layer2;float a;
    switch(direction) {
        case 'n':
            if(node == node->parent->nw) {
                a = neighborValue(root, node, 'w', layer2);
                if(layer1 == layer2)
                    return a*0.75f + node1->value*0.25f;
                else 
                    return a*0.6666666f + node1->value*0.3333333f;
            }
            if(node == node->parent->ne) {
                a = neighborValue(root, node, 'e', layer2);
                if(layer1 == layer2)
                    return a*0.75f + node1->value*0.25f;
                else 
                    return a*0.6666666f + node1->value*0.3333333f;
            }
        case 's':
            if(node == node->parent->sw) {
                a = neighborValue(root, node, 'w', layer2);
                if(layer1 == layer2)
                    return a*0.75f + node1->value*0.25f;
                else 
                    return a*0.6666666f + node1->value*0.3333333f;
            }
            if(node == node->parent->se) {
                a = neighborValue(root, node, 'e', layer2);
                if(layer1 == layer2)
                    return a*0.75f + node1->value*0.25f;
                else 
                    return a*0.6666666f + node1->value*0.3333333f;
            }
        case 'e':
            if(node == node->parent->ne) {
                a = neighborValue(root, node, 'n', layer2);
                if(layer1 == layer2)
                    return a*0.75f + node1->value*0.25f;
                else 
                    return a*0.6666666f + node1->value*0.3333333f;
            }
            if(node == node->parent->se) {
                a = neighborValue(root, node, 's', layer2);
                if(layer1 == layer2)
                    return a*0.75f + node1->value*0.25f;
                else 
                    return a*0.6666666f + node1->value*0.3333333f;
            }
        case 'w':
            if(node == node->parent->sw) {
                a = neighborValue(root, node, 's', layer2);
                if(layer1 == layer2)
                    return a*0.75f + node1->value*0.25f;
                else 
                    return a*0.6666666f + node1->value*0.3333333f;
            }
            if(node == node->parent->nw) {
                a = neighborValue(root, node, 'n', layer2);
                if(layer1 == layer2)
                    return a*0.75f + node1->value*0.25f;
                else 
                    return a*0.6666666f + node1->value*0.3333333f;
            }
    }
}

void insertObstacle(quadTreeNode* root, circle* obstacle) {
    // or we can use cache
    queue<quadTreeNode*> q; 
    q.push(root);
    while(!q.empty()) {
        quadTreeNode* head = q.front();
        q.pop();
        // rectangle* a = head->rect;
        if(obstacle->intersect(head) == 0) {
            if(!head->isLeaf) {
                if(head->nw) q.push(head->nw);
                if(head->ne) q.push(head->ne);
                if(head->sw) q.push(head->sw);
                if(head->se) q.push(head->se);
            } else {
                if(head->size > MINSIZE) {
                    refineRegion(head);
                    q.push(head->nw);
                    q.push(head->ne);
                    q.push(head->sw);
                    q.push(head->se);
                }
            }
        }
    }
}



void deleteBelowNode(quadTreeNode* head) {
    if(head == NULL || head->isLeaf) return;
    
    if(!(head->nw)->isLeaf) 
        deleteBelowNode(head->nw);
    delete(head->nw);
    head->nw = NULL;

    if(!(head->ne)->isLeaf)
        deleteBelowNode(head->ne);
    delete(head->ne);
    head->ne = NULL;

    if(!(head->sw)->isLeaf) 
        deleteBelowNode(head->sw);
    delete(head->sw);
    head->sw = NULL;
    
    if((head->se)->isLeaf) 
        deleteBelowNode(head->se);
    delete(head->se);
    head->se = NULL;
    
    head->isLeaf = true;
}

void movingObstacle(quadTreeNode* root, circle* cir1, circle* cir2) {    
    queue<quadTreeNode*> q;
    root = findCommonAncesstor(root, cir1, cir2);
// cout << "hi" << endl;
    q.push(root);

    while(!q.empty()) {
        quadTreeNode* head = q.front();
        q.pop();
// cout << "(" << head->xc << "," <<head->yc<< ")" << head->size <<  endl;
        if(!head->isLeaf) {
            // if(cir1->intersect(head) != 0) {  //和circle1不相交，子节点也不相交
                // if(head->nw) q.push(head->nw); 
                // if(head->ne) q.push(head->ne);
                // if(head->sw) q.push(head->sw);
                // if(head->se) q.push(head->se);
            // } else if (cir2->intersect(head) != 0) {
            if(cir1->intersect(head) == 0 && cir2->intersect(head) != 0){
                deleteBelowNode(head);
            } else { // 都相交，
                if(head->nw) q.push(head->nw);
                if(head->ne) q.push(head->ne);
                if(head->sw) q.push(head->sw);
                if(head->se) q.push(head->se);
            }
        } else { //if(head->isLeaf) 
            if(cir1->intersect(head) != 0 && cir2->intersect(head) == 0) {
                if(head->size > MINSIZE) {
                    refineRegion(head);
                    q.push(head->nw);
                    q.push(head->ne);
                    q.push(head->sw);
                    q.push(head->se);
                }
                // } else {
                    // cout << "   size too small to refine\n";
            // } else if(cir1->intersect(head) == 0 && cir2->intersect(head) != 0){
                // if(cir2->intersect(head->parent) != 0) {
               
   
                // }
                // } else {
                // cout << "    unchanged\n"; 
            }
        }
    }
    cout << "finish\n";
}

void quadTreeNodeDraw(quadTreeNode* node) {
    GLfloat x = node->xc, y = node->yc, a = node->size/2 - 0.01;
    glColor3f(1.0,1.0,1.0);

    glBegin(GL_QUADS);
        glVertex2f(x - a, y - a);
        glVertex2f(x - a, y + a);
        glVertex2f(x + a, y + a);
        glVertex2f(x + a, y - a);
    glEnd();
}

int mainWindow;
bool status = false;
bool statuss = false;
bool statusss = false;
quadTreeNode* root;
circle* cir;
quadTreeNode* click;

//------------------------------------------
//
void mymouse(int button, int state, int x, int y) {


    if(state==GLUT_DOWN) {
        GLint viewport[4]; 
        GLfloat winX, winY, winZ;
        GLdouble posX, posY, posZ;
        GLdouble modelview[16]; 
        GLdouble projection[16];

        glGetIntegerv(GL_VIEWPORT, viewport); 
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview); 
        glGetDoublev(GL_PROJECTION_MATRIX, projection);

        winX = x;
        winY = viewport[3] - (float)y;
        winZ = 0.f;
        // glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ); 
        gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
        
        cout << "click at (" << posX << "," << posY << ")\n";
        // cir = new circle(posX, posY, 0.01f);
        circle* cir1 = new circle(cir->xc, cir->yc - 0.1, cir->r);
        movingObstacle(root, cir, cir1);
        cir = cir1;
        // click = findEnclosure(root, cir);
                // glColor3f(1.0, 1.0, 1.0);
        // cout << root->size << endl;
        // cout << isContain(root->nw, cir) << endl;
        // 
        
        // click = findEnclose(root, cir);
        // circle* cir0 = new circle(0.125,0.125, 0.01f);
        // click = findCommonAncesstor(root, cir, cir0);
        // insertObstacle(root, cir);
        // deleteBelowNode(click);
        // click = findNeighbor(root, click, 'w');


        // cout << (click == NULL) << endl;
        // cout << click->size << endl;
        statuss = true;
    }
}

void intereact(unsigned char key, int xx, int yy) {
    // char x = 'a';
    if (key == 27) {
        glutDestroyWindow(mainWindow);
        exit(0);
    }
    else if (key == 97) {
        status = true;
        
    } else if (key == 98) {
        insertObstacle(root, cir);
        statuss = true;
    }
}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);  
            glColor3f(1.0, 1.0, 1.0);
    if(status) {
        queue<quadTreeNode*> q;
        q.push(root);
        while(!q.empty()) {
            quadTreeNode* head = q.front();
            q.pop();
            quadTreeNodeDraw(head);
            {
                if(head->nw) q.push(head->nw);
                if(head->ne) q.push(head->ne);
                if(head->sw) q.push(head->sw);
                if(head->se) q.push(head->se);
            }
        }
        cir->circleDraw();
    }

    if(statuss) {
        // glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
        cir->circleDraw();
        // quadTreeNodeDraw(click);
    }

    if(statusss) {
        glColor3f(1.0,0.0,0.0);
        quadTreeNodeDraw(click);
    }

    glutSwapBuffers();
}



int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800,800);
    mainWindow = glutCreateWindow("Lighthouse3D - GLUT Tutorial");
    root = new quadTreeNode(0.0f, 0.0f, 1.0f);
    // root = new quadTreeNode(-0.375f, 0.375f, 0.25f);
    cir = new circle(0,0.05,0.2);
    quadTree(root,2);
    insertObstacle(root, cir);
    
    glutKeyboardFunc(intereact);
    glutDisplayFunc(render);

    glutIdleFunc(render);
    glutMouseFunc(mymouse);
    glutMainLoop();
    return 1;
}



