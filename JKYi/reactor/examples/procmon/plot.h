#include"JKYi/noncopyable.h"
#include"JKYi/Types.h"

#include<vector>
#include<stdlib.h>
#include<string>

typedef struct gdImageStruct* gdImagePtr;

//封装好的画图类
class Plot : JKYi::Noncopyable{
public:
    Plot(int width,int heigth,int totalSeconds,int samplingPeriod);
    ~Plot();

    std::string plotCpu(const std::vector<double>& data);   //传入的数组中记录的就是cpu的使用率
private:
    std::string toPng();

    int getX(ssize_t x,ssize_t total)const;
    int getY(double value)const;
    void label(double maxValue);

    struct MyGdFont;
    typedef struct MyGdFont* MyGdFontPtr;

    const int width_;         //矩形的宽
    const int height_;        //矩形的高
    const int totalSeconds_;
    const int samplingPeriod_;
    gdImagePtr const image_;  //图片
    MyGdFontPtr const font_;  //字体
    const int fontWidth_;     //字体的宽
    const int fontHeight_;    //字体的高
    const int background_;    //图片的背景颜色
    const int black_;         //画出的线的颜色
    const int gray_;          
    const int blue_;

    const int kRightMargin_;
    static const int kLeftMargin_ = 5;
    static const int kMarginY_ = 5;

    const double ratioX_;
};
