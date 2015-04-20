#include <string>
#include <vector>
#include <iostream>
#include <v8.h>
#include <node.h>
#include <node_buffer.h>

using namespace std;
using namespace v8;
using namespace node;

const long MAX_PUSH_COUNT_ONE_DAY =  50000L;

typedef struct sellerInfo {
    int sellerID;
    int historyOrderCnt;
    int todayOrderCnt;
    int todayPushCnt;
    float historyRate;
    float pushRate;
    float alpha;
    float beta;
    float grade;
}SI;

typedef struct sellerNode {
    SI seller;
    sellerNode *pre;
    sellerNode *next;
}SInode;

class sellerActiveGrade {
    public:
        sellerActiveGrade();
        ~sellerActiveGrade();

        SInode *insert_seller(int id, int h,int t,int p);
        SInode *get_active_grade();
    private:
        SInode  *pSeller;
        SInode *data_normalization();
        SInode *grade_sort(SInode *dst);
        int calc_alpha();
        int calc_beta();
};

sellerActiveGrade::sellerActiveGrade(){
    pSeller = new SInode;
    pSeller->pre = NULL;
    pSeller->next = NULL;
}

sellerActiveGrade::~sellerActiveGrade(){
    SInode *tmp = pSeller;
    SInode *next = tmp->next;
    while(tmp != NULL){
        delete tmp;
        tmp = next;
        next = next->next;
    }

}

SInode *sellerActiveGrade::insert_seller(int id, int h,int t,int p){
    SInode *newSI = new SInode;
    newSI->seller.sellerID = id;
    newSI->seller.historyOrderCnt = h;
    newSI->seller.todayOrderCnt = t;
    newSI->seller.todayPushCnt = p;
    newSI->next = NULL;
    newSI->pre = NULL;
    if(pSeller->pre == NULL){
        pSeller = newSI;
        pSeller->pre = pSeller;

    }else{
        newSI->pre = newSI;
        newSI->next = pSeller;
        pSeller->pre = newSI;
        pSeller = newSI;
    }

    //	cout<<"insert sellerNode:" << "id=" << id << " h=" << h << " t=" << t << " p="<< p << endl;
    SInode *ps = pSeller;
    //	while(ps!= NULL){
    //		cout << " current:" << ps->seller.sellerID << " " << ps->seller.historyOrderCnt << " " << ps->seller.todayOrderCnt << " " << ps->seller.todayPushCnt << endl;	
    //		ps = ps->next;
    //	}

    return pSeller;
}

SInode *sellerActiveGrade::data_normalization(){
    int minH = pSeller->seller.historyOrderCnt;
    int maxH = pSeller->seller.historyOrderCnt;
    int	maxP = pSeller->seller.todayPushCnt;
    int minP = maxP;

    SInode *tmpSInode = pSeller->next;

    while(tmpSInode != NULL){
        if(tmpSInode->seller.historyOrderCnt > maxH){
            maxH = tmpSInode->seller.historyOrderCnt;
        }
        if(tmpSInode->seller.todayPushCnt > maxP){
            maxP = tmpSInode->seller.todayPushCnt;
        }

        tmpSInode = tmpSInode->next;
    }

    tmpSInode = pSeller;

    cout << "maxH = " << maxH << "; maxP= " << maxP <<endl;
    while(tmpSInode != NULL){
        tmpSInode->seller.historyRate = (float)tmpSInode->seller.historyOrderCnt/maxH;
        tmpSInode->seller.pushRate = (float)tmpSInode->seller.todayPushCnt/maxP;
        tmpSInode = tmpSInode->next;
    }

    return pSeller;
}



int sellerActiveGrade::calc_alpha(){
    SInode *tmpSInode = pSeller;

    while(tmpSInode != NULL){
        if(tmpSInode->seller.todayPushCnt == 0){
            tmpSInode->seller.alpha = 1;
            tmpSInode = tmpSInode->next;
            continue;
        }
        tmpSInode->seller.alpha = (float)tmpSInode->seller.todayOrderCnt / tmpSInode->seller.todayPushCnt;
        tmpSInode = tmpSInode->next;
    }
    return 0;
}

int sellerActiveGrade::calc_beta(){
    SInode *tmpSInode = pSeller;

    while(tmpSInode != NULL){
        if(tmpSInode->seller.pushRate == 0){
            tmpSInode->seller.beta = MAX_PUSH_COUNT_ONE_DAY;
            tmpSInode = tmpSInode->next;
            continue;
        }
        tmpSInode->seller.beta = (float)tmpSInode->seller.alpha / tmpSInode->seller.pushRate;
        tmpSInode = tmpSInode->next;
    }
    return 0;
}

SInode *sellerActiveGrade::grade_sort(SInode *dst){
    if(dst == NULL) return pSeller;

#if 0
    cout << "sort begin:"<<endl;
    SInode *gradeSeller = dst;
    while(gradeSeller!= NULL){
        cout << "grade Seller >> "  << gradeSeller->seller.sellerID << " " << gradeSeller->seller.historyOrderCnt << " " << gradeSeller->seller.todayOrderCnt << " " << gradeSeller->seller.todayOrderCnt << " ? " << gradeSeller->seller.grade << endl;
        gradeSeller = gradeSeller->next;
    }
#endif

    SInode *firstSInode = dst;
    SInode *nextSInode = firstSInode->next;
    SInode *tmpSInode = firstSInode;
    while(nextSInode != NULL){
        if(firstSInode->seller.grade < nextSInode->seller.grade){

            SInode *fp = firstSInode->pre;
            SInode *fn = firstSInode->next;

            SInode *np = nextSInode->pre;
            SInode *nn = nextSInode->next;
            //cout << firstSInode->seller.sellerID << " lt " << nextSInode->seller.sellerID << endl;
            if(nn != NULL){
                nextSInode->next->pre = firstSInode;
            }

            if(fp != firstSInode){
                fp->next = nextSInode;
            }

            tmpSInode = nextSInode;

            nextSInode = firstSInode;
            firstSInode = tmpSInode;
            if(np == nextSInode){
                //相邻两个元素
                nextSInode->pre = firstSInode;
                nextSInode->next = nn;
                firstSInode->next = nextSInode;

            }else{
                //不相邻的两个元素
                nextSInode->pre = np;
                np->next = nextSInode;
                nextSInode->next = nn;
                firstSInode->next = fn;
                fn->pre = firstSInode;
            }

            if(fp == nextSInode){
                firstSInode->pre = firstSInode;
            }else{
                firstSInode->pre = fp;
            }

            nextSInode = nn;

        }else{
            //cout << firstSInode->seller.sellerID << " gt " << nextSInode->seller.sellerID << endl;
            nextSInode	= nextSInode->next;
        }

    }

#if 0
    cout << "grade sort end:"<<endl;
    gradeSeller = firstSInode;
    while(gradeSeller!= NULL ){
        cout << "grade Seller >> "  << gradeSeller->seller.sellerID << " " << gradeSeller->seller.historyOrderCnt << " " << gradeSeller->seller.todayOrderCnt << " " << gradeSeller->seller.todayOrderCnt << " ? " << gradeSeller->seller.grade << endl;
        gradeSeller = gradeSeller->next;
    }
#endif
    grade_sort(firstSInode->next);
    return firstSInode;
}

SInode *sellerActiveGrade::get_active_grade(){
    data_normalization();
    calc_alpha();
    calc_beta();


    SInode *tmpSInode = pSeller;

    while(tmpSInode != NULL){
        tmpSInode->seller.grade = tmpSInode->seller.beta + tmpSInode->seller.historyRate;
        tmpSInode = tmpSInode->next;
    }

    SInode *ps = pSeller;
    while(ps!= NULL){
        cout << " after:ID=" << ps->seller.sellerID << " h=" << ps->seller.historyOrderCnt << " t=" << ps->seller.todayOrderCnt << " p=" << ps->seller.todayPushCnt  << " h1=" << ps->seller.historyRate << " p1=" << ps->seller.pushRate << " alpha="  << ps->seller.alpha << " beta=" << ps->seller.beta << " grade=" << ps->seller.grade << endl;	
        ps = ps->next;

    }
    // 排序
    grade_sort(pSeller);

    while(pSeller->pre != pSeller){
        pSeller = pSeller->pre;
    }
    return pSeller;

}



Handle <Value> sort_seller_by_grade(const Arguments &args){

    HandleScope scope;

    /*
       Handle <Object> v8obj = Object::new();
       v8obj->Set(String::NewSymbol("historyOrderCnt"),Integer::New(1));
       v8obj->Set(String::NewSymbol("todayOrderCnt"),Integer::New(1));
       v8obj->Set(String::NewSymbol("todayPushCnt"),Integer::New(1));
       v8obj->Set(String::NewSymbol("sellerID"),Integer::New(1));

       if(v8obj->Has(String::New("historyOrderCnt"))) {
       Handle <Value> value = v8obj->Get(String::New("historyOrderCnt"));
       }
       */
    sellerActiveGrade *pSellerGrade = new sellerActiveGrade();

    Handle<Array> array = Handle<Array>::Cast(args[0]);

    for(int i=0; i<array->Length(); i=i+4){
        int id = array->Get(i)->ToInteger()->Value();
        int h = array->Get(i+1)->ToInteger()->Value();
        int t = array->Get(i+2)->ToInteger()->Value();
        int p = array->Get(i+3)->ToInteger()->Value();
        pSellerGrade->insert_seller(id,h,t,p);

    }
    SInode *gradeSeller = pSellerGrade->get_active_grade();
#if 1
    cout << "gradeSeller end:"<<endl;
    while(gradeSeller!= NULL){
        cout << "grade Seller >> "  << gradeSeller->seller.sellerID << " " << gradeSeller->seller.historyOrderCnt << " " << gradeSeller->seller.todayOrderCnt << " " << gradeSeller->seller.todayPushCnt << " ? " << gradeSeller->seller.grade << endl;
        gradeSeller = gradeSeller->next;
    }
#endif
    //return scope.Close(pSellerGrade);
    return scope.Close(Integer::New(100));

}


void init(Handle <Object> target){
    target->Set(String::NewSymbol("sort_seller_by_grade"),FunctionTemplate::New(sort_seller_by_grade)->GetFunction());
}

NODE_MODULE(seller_grade, init);
