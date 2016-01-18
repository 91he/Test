#include <stdio.h>
#include <string.h>
#include <stdlib.h>

template<typename T>
class MyQueue{
private:
    int start;
    int end;
    int size;
    int count;
    T   *arr;
public:
    MyQueue():start(0), end(0), size(8), count(0){
        arr = new T[size];//RtlAlloc
    }
    void push(T data){
        if(count == size){
            int i = 0;
            int t = count;
            int ts = size * 2;
            T* tmp = new T[ts];//Rtl
            while(!empty()){
                tmp[i++] = front();
                pop();
            }
            delete[] arr;//RtlFree
            count = t;
            size = ts;
            arr = tmp;
            start = 0;
            end = count;
        }
        count++;
        arr[end++] = data;
        if(end == size) end = 0;
    }
    void pop(){
        count--;
        if(++start == size) start = 0;
    }
    T front(){
        return arr[start];
    }
    T back(){
        if(!end) return arr[size - 1];
        return arr[end - 1];
    }
    bool empty(){
        return count == 0;
    }
};

template<typename N, typename B>
class MyMapIterator{
    B *base;
    int cur_pos;
public:
    MyMapIterator():base(NULL), cur_pos(0){}
    MyMapIterator(B* base, int pos):base(base), cur_pos(pos){}
    N operator*(){
        int i = 0;
        for(int j = 0; j != cur_pos; i++){
            if(base->find(i)) j++;
        }
        return base->at(i - 1);
    }

    MyMapIterator<N, B> operator++(int){
        MyMapIterator<N, B> ret = *this;
        cur_pos++;
        return ret;
    }
    bool operator==(MyMapIterator<N, B> other){
        return base == other.base && cur_pos == other.cur_pos;
    }
    bool operator!=(MyMapIterator<N, B> other){
        return !operator==(other);
    }
};

template<typename N>
class IdMap{
    int size;
    N *arr;
    bool *has;
    int count;
public:
    typedef MyMapIterator<N, IdMap<N> > iterator;
    IdMap():size(24), arr(NULL), has(NULL), count(0){
        //arr = new N[size];//RtlAlloc
        arr = (N*)malloc(size * sizeof(N));//RtlAlloc
        has = new bool[size];//RtlAlloc
        bzero(has, size * sizeof(bool));
    };
    N &operator[](int id){
        if(id >= size){
            int dsize = size;
            while(dsize < id) dsize *= 2;

            N* tmp = (N*)malloc(dsize * sizeof(N));//RtlAlloc
            memcpy(tmp, arr, size * sizeof(N));
            free(arr);
            size = dsize;

            bool *th = (bool*)malloc(dsize * sizeof(bool));
            bzero(th, dsize * sizeof(bool));
            memcpy(th, has, size * sizeof(N));
            free(has);
            has = th;
        }
        count++;
        has[id] = true;
        return arr[id];
    }
    N at(int id){
        if(id >= size) return N();
        return arr[id];
    }
    bool find(int id){
        if(id >= size) return false;
        return has[id];
    }
    iterator erase(iterator it){
        count--;
        for(int i = 0, j = 0; i < size; i++){
            if(has[i]) j++;
            if(j == it.cur_pos){
                has[i] = false;
                break;
            }
        }

        return it;
    }
    void erase(int id){
        count--;
        has[id] = false;
    }

    iterator begin(){
        return iterator(this, 1);
    }
    iterator end(){
        return iterator(this, count + 1);
    }
};

typedef enum{
    PROG_FREE,
    PROG_PROCESSING,
}PROC_PROG;


struct WaveHeader{
    int type;
    int val1;
    int val2;
    int val3;
};

struct WaveData{
    int len;
    char *buf;
};

class MyStream{
    bool initHeader;
    PROC_PROG curProgress;
    int procLen;
    char *procBuffer;
    WaveHeader waveHeader;
    MyQueue<WaveData> waveQueue;
public:
    MyStream():curProgress(PROG_FREE), procLen(0), procBuffer(NULL){}
    void clear(){
        while(!waveQueue.empty()){
            WaveData data = waveQueue.front();
            free(data.buf);//TODO:
            waveQueue.pop();
        }
    }
    void setHeader(){
        initHeader = true;
        procLen = sizeof(waveHeader);
        procBuffer = (char*)&waveHeader;
        clear();
    }
    void setData(WaveData data){
        waveQueue.push(data);
    }
    bool empty(){
        return waveQueue.empty();
    }
    PROC_PROG prog(){
        return curProgress;
    }
    int copy(char *dest, int len){
        int ret = procLen;

        if(!proclen) return 0;

        if(len >= procLen){
            char *start;
            char *end;
            WaveData data = waveQueue.front();

            memcpy(dest, procBuffer, procLen);//TODO

            start = data.buf;
            end = start + sizeof(waveHeader);
            if(procBuffer >= start && procBuffer < end){
                free(data.buf);//TODO
                waveQueue.pop();
                data = waveQueue.front();
            }

            procBuffer = data.buf;
            procLen = data.len;
            curProgress = PROG_FREE;
        }else{
            memcpy(dest, procBuffer, len);//TODO
            procLen -= len;
            procBuffer +=  len;
            curProgress = PROG_PROCESSING;
            ret = len;
        }

        return ret;
    }
};

typedef int PIRQ;
class MsgWorker{
    bool connecting;
    int curStreamId;
    int activeStreamId;
    MyQueue<PIRQ> irqs;//TODO
    IdMap<MyStream*> streams;
public:
    MsgWorker():connecting(false), curStreamId(-1), activeStreamId(-1){}
    void processIRQ(PIRQ pIrq){
        MyStream *stream = streams[curStreamId];
        if(stream->prog() == PROG_FREE && curStreamId != activeStreamId){
            streams[curStreamId]->clear();
            curStreamId = activeStreamId;
            stream = streams[curStreamId];
        }
#if 0
        char *buf = MmGetMdlVirtualAddress(pIrq->MdlAddress);
        int len = MmGetMdlByteCount(pIrq->MdlAddress);
        int ret = stream->copy(buf, len);
        pIrq->IoStatus.Information = ret;
        if(ret){
            pIrq->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        }else{
            pIrq->IoStatus.Status = STATUS_PENDING;
            irqs.push(pIrq);
        }
#endif
    }
    void processHeader(int id, WaveHeader header){
        MyStream *stream = NULL;
        if(!irqs.empty()){
            PIRQ pIrq = irqs.front();
#if 0
            char *dest = MmGetMdlVirtualAddress(pIrq->MdlAddress);
            int count = MmGetMdlByteCount(pIrq->MdlAddress);
            if(len > count){
                //save data
            }
            //TODO: make & send

            pIrp->IoStatus.Infomation = ;
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
#endif
            irqs.pop();
        }else{
            if(!streams.find(id)){
                stream = new MyStream();//TODO
                streams[id] = stream;
            }else{
                stream = streams[id];
            }
            //TODO: save header;
            //stream->setHeader();
        }
    }
    void processData(int id, char *buf, int len){
        MyStream *stream = NULL;
        if(irqs.empty()){
            if(!streams.find(id)){
                stream = new MyStream();//TODO
                streams[id] = stream;
            }else{
                stream = streams[id];
            }
            WaveData data;
            data.len = len;
            data.buf = (char*)malloc(len);
            //TODO: make buf
            stream->setData(data);
        }else{
            PIRQ pIrq = irqs.front();
            //TODO: copy & send
#if 0
            char *dest = MmGetMdlVirtualAddress(pIrq->MdlAddress);
            int count = MmGetMdlByteCount(pIrq->MdlAddress);
            if(len > count){
                //TODO: save buf: buf+count, len: len - count;
                WaveData data;
                data.len = len;
                data.buf = (char*)malloc(len);
                //TODO: make buf
                stream->setData();
            }else{
                count = len;
            }
            //TODO: make buf
            memcpy(dest, buf, count);
            
            pIrp->IoStatus.Infomation = count;
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
#endif
            irqs.pop();
        }
    }
};

int main(){
#if 1
    MyQueue<int> q;
    q.push(3);
    q.push(2);
    q.push(1);
    q.push(0);
    q.pop();
    q.pop();
    q.push(4);
    q.push(5);
    q.push(6);
    q.push(8);
    q.push(7);
    q.push(3);
    q.push(5);
    q.push(4);

    while(!q.empty()){
        printf("xxxx%d\n", q.front());
        q.pop();
    }
#endif
    IdMap<int> map;
    IdMap<int>::iterator it;//= map.end();
    //IdMap<int>::iterator itt = map.begin();
    //itt.operator==(it);
    //itt == it;
    //it.operator==(map.begin());
    map[3] = 5;
    //printf("%d\n", map.at(3));
    map[5] = 2;
    map[0] = 7;
    map[9] = 6;
    for(it = map.begin(); it != map.end(); it++){
        printf("%d\n", *it);
    }

    return 0;
}
