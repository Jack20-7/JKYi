#include"JKYi/application.h"

int main(int argc,char ** argv){
    JKYi::Application app;
    if(app.init(argc,argv)){
        return app.run();
    }
    return 0;
}
