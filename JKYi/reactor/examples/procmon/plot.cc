#include"JKYi/reactor/examples/procmon/plot.h"
#include"/usr/local/include/gd/gd.h"
#include"/usr/local/include/gd/gdfonts.h"

#include<algorithm>
#include<math.h>

struct Plot::MyGdFont : public gdFont{};

Plot::Plot(int width,int height,int totalSeconds,int samplingPeriod)
    :width_(width),
     height_(height),
     totalSeconds_(totalSeconds),
     samplingPeriod_(samplingPeriod),
     image_(gdImageCreate(width_,height_)),
     font_(static_cast<MyGdFont*>(gdFontGetSmall())),
     fontWidth_(font_->w),
     fontHeight_(font_->h),
     //添加一些即将需要用到的颜色
     background_(gdImageColorAllocate(image_,255,255,240)), 
     black_(gdImageColorAllocate(image_,0,0,0)),            
     gray_(gdImageColorAllocate(image_,200,200,200)),
     blue_(gdImageColorAllocate(image_,128,128,128)),

     kRightMargin_(3 * fontWidth_ + 5),
     ratioX_(static_cast<double>(samplingPeriod_ * 
                                (width_ - kLeftMargin_ - kRightMargin_)) / totalSeconds_){
}

Plot::~Plot(){
    gdImageDestroy(image_);
}

std::string Plot::plotCpu(const std::vector<double>& data){
    //画一个矩形并且填充
    //矩形的长是width，高是height
    gdImageFilledRectangle(image_,0,0,width_,height_,background_);

    if(data.size() > 1){
        //给图形设置线宽
        gdImageSetThickness(image_,2);

        double max = *std::max_element(data.begin(),data.end());
        if(max >= 10.0){
            //ceil返回max向上取整的整数
            max = ceil(max);
        }else{
            max = std::max(0.1 , ceil(max * 10.0) / 10.0);
        }
        label(max);

        for(size_t i = 0;i < data.size() - 1;++i){
            gdImageLine(image_,
                        getX(i,data.size()),
                        getY(data[i] / max),
                        getX(i + 1,data.size()),
                        getY(data[i + 1] / max),
                        black_);
        }
    }
    int total = totalSeconds_ / samplingPeriod_;
    gdImageSetThickness(image_,1);
    //gdImageLine用来在两点之间绘制直线
    gdImageLine(image_,getX(0,total),getY(0) + 2,getX(total,total),getY(0) + 2,gray_);
    gdImageLine(image_,getX(total,total),getY(0) + 2,getX(total,total),getY(1) + 2,gray_);

    return toPng();
}


//maxValue 表示的是最大CPU使用率
void Plot::label(double maxValue){
    char buf[64];
    if(maxValue >= 10.0){
        snprintf(buf,sizeof buf,"%.0f",maxValue);
    }else{
        snprintf(buf,sizeof buf,"%.1f",maxValue);
    }

    //用来在画布上写字符,这里也就是在矩形上下数字
    gdImageString(image_,
                  font_,
                  width_ - kRightMargin_ + 3,            //起始x
                  kMarginY_ - 3,                         //起始y
                  reinterpret_cast<unsigned char*>(buf), //写的内容
                  black_);                               //颜色

    snprintf(buf,sizeof buf,"0");
    gdImageString(image_,
                  font_,
                  width_ - kRightMargin_ + 3,
                  height_ - kMarginY_ - 3 - fontHeight_ / 2,
                  reinterpret_cast<unsigned char*>(buf),
                  gray_);

    snprintf(buf,sizeof buf,"-%ds",totalSeconds_);
    gdImageString(image_,
                  font_,
                  kLeftMargin_,
                  height_ - kMarginY_ - fontHeight_,
                  reinterpret_cast<unsigned char*>(buf),
                  blue_);
}

int Plot::getX(ssize_t i,ssize_t total)const{
    double x = (width_ - kLeftMargin_ - kRightMargin_) + 
                               static_cast<double>(i - total) * ratioX_;
    return static_cast<int>(x + 0.5) + kLeftMargin_;
}

int Plot::getY(double value)const{
    return static_cast<int>((1.0 - value) * (height_ - 2 * kMarginY_) + 0.5) + kMarginY_;
}

std::string Plot::toPng(){
    int size = 0;
    void* png = gdImagePngPtr(image_,&size);
    std::string result(static_cast<char*>(png),size);

    gdFree(png);

    return result;
}
