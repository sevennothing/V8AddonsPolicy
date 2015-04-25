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
const int  PUSH_COMMON_PARAM = 4;
const float ACTIVE_CARE = 0.5;

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
        int    data_normalization();
        SInode *grade_sort(SInode *dst);

        int calc_alpha();
        int calc_beta();
        float calc_gama(int maxValue, int dstValue);   //推送权重补偿
};

sellerActiveGrade::sellerActiveGrade(){
	cout<< "sellerActiveGrade()"<<endl;
    pSeller = new SInode;
    pSeller->pre = NULL;
    pSeller->next = NULL;
}

sellerActiveGrade::~sellerActiveGrade(){
	cout<< "~sellerActiveGrade()"<<endl;
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

int sellerActiveGrade::data_normalization(){
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
        if(maxH == 0){
            tmpSInode->seller.historyRate = 1;
        }else{
            tmpSInode->seller.historyRate = (float)tmpSInode->seller.historyOrderCnt/maxH;
        }
        #if 0
        if(maxP == 0){
            tmpSInode->seller.pushRate = 1;
        }else{
            tmpSInode->seller.pushRate = (float)tmpSInode->seller.todayPushCnt/maxP;
        }
        #endif

        tmpSInode = tmpSInode->next;
    }

    return maxP;
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
        if(tmpSInode->seller.todayPushCnt == 0){
            tmpSInode->seller.beta = (ACTIVE_CARE * tmpSInode->seller.alpha) + 1;
            tmpSInode = tmpSInode->next;
            continue;
        }
        tmpSInode->seller.beta = ACTIVE_CARE * tmpSInode->seller.alpha;
        tmpSInode = tmpSInode->next;
    }
    return 0;
}

float sellerActiveGrade::calc_gama(int maxValue, int dstValue){
    return (float)(maxValue - dstValue) / PUSH_COMMON_PARAM;
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
    int maxPushCnt = data_normalization();
    calc_alpha();
    calc_beta();

    SInode *tmpSInode = pSeller;

    while(tmpSInode != NULL){
        // gama值作为推送补偿，补偿系数为 PUSH_COMMON_PARAM
        float gama = calc_gama(maxPushCnt,tmpSInode->seller.todayPushCnt);
        tmpSInode->seller.grade = tmpSInode->seller.beta + tmpSInode->seller.historyRate + gama;
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
    sellerActiveGrade *pSellerGrade = new sellerActiveGrade();

    Handle<Array> array = Handle<Array>::Cast(args[0]);
    //确保参数是回调函数，是函数
    if (!args[1]->IsFunction()) {
	  return ThrowException(Exception::TypeError(
			  String::New("Second argument must be a callback function")));
	}
	Local<Function> callback = Local<Function>::Cast(args[1]);

    for(int i=0; i<array->Length(); i=i+4){
        int id = array->Get(i)->ToInteger()->Value();
        int h = array->Get(i+1)->ToInteger()->Value();
        int t = array->Get(i+2)->ToInteger()->Value();
        int p = array->Get(i+3)->ToInteger()->Value();
        pSellerGrade->insert_seller(id,h,t,p);

    }
    SInode *gradeSeller = pSellerGrade->get_active_grade();
#if 1
	SInode *pgs = gradeSeller;
    cout << "gradeSeller end:"<<endl;
    while(pgs!= NULL){
        cout << "grade Seller >> "  << pgs->seller.sellerID << " " << pgs->seller.historyOrderCnt << " " << pgs->seller.todayOrderCnt << " " << pgs->seller.todayPushCnt << " ? " << pgs->seller.grade << endl;
        pgs = pgs->next;
    }
#endif
	int length = 0;
	SInode *p = gradeSeller;
	while(p != NULL){
		length++;
		p = p->next;
	}	

	//Local<Value> sortSeller = Local<Array>::New(length);
	Local<Array> sortSeller;
	sortSeller = sortSeller->New(length);
	cout << "length=" << length << " ; "<< sortSeller->Length()<<endl;
	p = gradeSeller;
	int i=0;
	while(p != NULL){
		sortSeller->Set(i,Integer::New(p->seller.sellerID));
		p = p->next;
		i++;
	}

	const unsigned argc = 1;
	Local<Value> result[argc] = {sortSeller};	
	callback->Call(Context::GetCurrent()->Global(), argc, result);

    return scope.Close(sortSeller);

}


void init(Handle <Object> target){
    target->Set(String::NewSymbol("sort_seller_by_grade"),FunctionTemplate::New(sort_seller_by_grade)->GetFunction());
}

NODE_MODULE(seller_grade, init);
