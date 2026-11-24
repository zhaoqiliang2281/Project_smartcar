/*
 * Camera.c
 *
 *  Created on: 2025年2月9日
 *      Author: 皮祖忻
 */
#include "zf_common_headfile.h"
#include "Camera.h"

//放在main函数里，这里只是没地方先放这里
//           if(mt9v03x_finish_flag)
//               {
//                   Threshold=My_Adapt_Threshold(mt9v03x_image[0],MT9V03X_W, MT9V03X_H);//大津算阈值
                 //  Image_Binarization(Threshold);//图像二值化

      //            ips200_displayimage03x((const uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H);                       // 显示原始图像
      //            ips200_show_gray_image(0, 0, (const uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H, 240, 180, Threshold);     // 显示二值化图像
//                    mt9v03x_finish_flag = 0;
//               }

int jump_flag=0;
uint8 display_image[LCDH][LCDW];              //用于处理的二值化图像
float offect = 0.0f,last_offect = 0.0f;
int ImageScanInterval;                         //扫边范围    上一行的边界+-ImageScanInterval
int ImageScanInterval_Cross;                   //270°的弯道后十字的扫线范围
uint8 Image_Use[LCDH][LCDW];          //灰度图像
uint8 Pixle[LCDH][LCDW];              //用于处理的二值化图像
static int Ysite = 0, Xsite = 0;                   //Y坐标=列
static uint8* PicTemp;                             //保存单行图像
static int IntervalLow = 0, IntervalHigh = 0;      //定义高低扫描区间
static int ytemp = 0;                              //存放行
static int TFSite = 0, FTSite = 0;                 //存放行
static float DetR = 0, DetL = 0;                   //存放斜率
static int BottomBorderRight = 79,                 //59行右边界
BottomBorderLeft = 0,                              //59行左边界
BottomCenter = 0;                                  //59行中点
ImageDealDatatypedef ImageDeal[60];                //记录单行的信息
ImageStatustypedef ImageStatus;                    //图像的全局变量
ImageStatustypedef ImageData;             ///////需要修改的图像阈值参数
SystemDatatypdef SystemData;
ImageFlagtypedef ImageFlag;
uint8 Ring_Help_Flag = 0;                      //进环辅助标志
int Left_RingsFlag_Point1_Ysite, Left_RingsFlag_Point2_Ysite;   //左圆环判断的两点纵坐标
int Right_RingsFlag_Point1_Ysite, Right_RingsFlag_Point2_Ysite; //右圆环判断的两点纵坐标
int Point_Xsite,Point_Ysite;                   //拐点横纵坐标
int Repair_Point_Xsite,Repair_Point_Ysite;     //补线点横纵坐标
int forklenth;
int barnlenth;
int ramplenth;
float Weighting[10] = {0.96, 0.92, 0.88, 0.83, 0.77,0.71, 0.65, 0.59, 0.53, 0.47};//10行权重参数，随意更改，基本不影响，大致按照正态分布即可
uint8 ExtenLFlag = 0;  //是否左延长标志
uint8 ExtenRFlag = 0;  //是否右延长标志
float ang_l;
int ycircle=0;
int xcircle=0;
int axcircle=0;
int aycircle=0;
int bxcircle=0;
int bycircle=0;

float K;
#define MAX_CONES   2
float32                            threshold;           /**< \brief Event window threshold in second. If 0, no event are generated */
uint8 Half_Road_Wide[60] =                      //直到赛道半宽
{  4, 5, 5, 6, 6, 6, 7, 7, 8, 8,
        9, 9,10,10,10,11,12,12,13,13,
       13,14,14,15,15,16,16,17,17,17,
       18,18,19,19,20,20,20,21,21,22,
       23,23,23,24,24,25,25,25,26,26,
       27,28,28,28,29,30,31,31,31,32,
};

uint8 Half_Bend_Wide[60] =                      //弯道赛道半宽
{   33,33,33,33,33,33,33,33,33,33,
    33,33,32,32,30,30,29,29,28,27,
    28,27,27,26,26,25,25,24,24,23,
    22,21,21,22,22,22,23,24,24,24,
    25,25,25,26,26,26,27,27,28,28,
    28,29,29,30,30,31,31,32,32,33,
};

uint8 buxianwide[60]={6,7,8,9,10,11,12,13,14,15,
                      16,17,18,19,20,21,22,23,24,25,
                      26,27,28,29,30,31,32,33,34,35,
                      36,37,38,39,40,41,42,43,44,45,
                      46,47,48,49,50,51,52,53,54,55,
                      56,57,58,59,60,61,62,63,64,65};

float Mh = MT9V03X_H;
float Lh = LCDH;
float Mw = MT9V03X_W;
float Lw = LCDW;

uint8_t visited[uLCDH][uLCDW] = {{0}}; // 全部初始化为0
Cone current_cone = {0}; // 初始化为全0，等效于 {0, 0, 0, 0}
int area = 0;
int start_x = 50;  // 起始x坐标
int start_y = 60;  // 起始y坐标
//Cone current_cone = {0, 0, 0, 0};  // 锥桶结构体 初始值全为0
int current_area = 0;  //初始化面积计数器

void image_data_init(void){
    //图像参数
    //adcsum = 0;
    ImageStatus.MiddleLine = 39;//中线
    ImageStatus.TowPoint_Gain = 0.2;
    ImageStatus.TowPoint_Offset_Max = 5;
    ImageStatus.TowPoint_Offset_Min = -2;
    ImageStatus.TowPointAdjust_v = 160;
    ImageStatus.Det_all_k = 0.7;  //待定自动补线斜率
    ImageStatus.CirquePass = 'F';
    ImageStatus.IsCinqueOutIn = 'F';
    ImageStatus.CirqueOut = 'F';
    ImageStatus.CirqueOff = 'F';
    ImageStatus.Barn_Flag = 0;
    ImageStatus.straight_acc = 0;

    ImageStatus.TowPoint =24;           //前瞻  ///
    ImageStatus.Threshold_static = 70;   //静态阈值  40-80
    ImageStatus.Threshold_detach = 220;  //阳光算法  亮斑分离140-220   //180
    ImageScanInterval = 2;               //扫边范围    上一行的边界+-ImageScanInterval
    ImageScanInterval_Cross = 2;         //十字扫线范围
    ImageStatus.variance_acc = 25;       //直道检测
    SystemData.clrcle_num=0;
    ImageStatus.newblue_flag=0;
    SystemData.Stop = 1;                 //启动标志位

}

void compressimage(void) {
  const int src_h = Mh, src_w = Mw;       // 源图像尺寸
  const int dst_h = LCDH, dst_w = LCDW;   // 目标尺寸
  uint8_t *dst_ptr = &Image_Use[0][0];    // 目标起始指针

  for (int y = 0; y < dst_h; y++) {
    // 计算源图像行（四舍五入）
    int src_row = (y * src_h * 2 + dst_h) / (2 * dst_h);
    if (src_row >= src_h) src_row = src_h - 1;

    const uint8_t *src_line = mt9v03x_image[src_row];
    for (int x = 0; x < dst_w; x++) {
      // 计算源图像列（四舍五入）
      int src_col = (x * src_w * 2 + dst_w) / (2 * dst_w);
      if (src_col >= src_w) src_col = src_w - 1;

      *dst_ptr++ = src_line[src_col];
    }
  }
  mt9v03x_finish_flag = 0;
}
//-------------------------------------------------------------------------------------------------------------------
//  @brief      优化的大津法
//  @param      image  图像数组
//  @param      clo    宽
//  @param      row    高
//  @param      pixel_threshold 阈值分离
//  @return     uint8
//  @since      2021.6.23
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
uint8 Threshold_deal(uint8* image,
                     uint16 col,
                     uint16 row,
                     uint32 pixel_threshold) {
#define GrayScale 256
  uint16 width = col;
  uint16 height = row;
  int pixelCount[GrayScale];
  float pixelPro[GrayScale];
  int i, j, pixelSum = width * height;
  uint8 threshold = 0;
  uint8* data = image;  //指向像素数据的指针
  for (i = 0; i < GrayScale; i++) {
    pixelCount[i] = 0;
    pixelPro[i] = 0;
  }

  uint32 gray_sum = 0;
  //统计灰度级中每个像素在整幅图像中的个数
  for (i = 0; i < height; i += 1) {
    for (j = 0; j < width; j += 1) {
      // if((sun_mode&&data[i*width+j]<pixel_threshold)||(!sun_mode))
      //{
      pixelCount[(
          int)data[i * width + j]]++;  //将当前的点的像素值作为计数数组的下标
      gray_sum += (int)data[i * width + j];  //灰度值总和
      //}
    }
  }

  //计算每个像素值的点在整幅图像中的比例
  for (i = 0; i < GrayScale; i++) {
    pixelPro[i] = (float)pixelCount[i] / pixelSum;
  }


  //遍历灰度级[0,255]
  float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
  w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
  for (j = 0; j < pixel_threshold; j++) {
    w0 +=
        pixelPro[j];  //背景部分每个灰度值的像素点所占比例之和 即背景部分的比例
    u0tmp += j * pixelPro[j];  //背景部分 每个灰度值的点的比例 *灰度值

    w1 = 1 - w0;
    u1tmp = gray_sum / pixelSum - u0tmp;

    u0 = u0tmp / w0;    //背景平均灰度
    u1 = u1tmp / w1;    //前景平均灰度
    u = u0tmp + u1tmp;  //全局平均灰度
    deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
    if (deltaTmp > deltaMax) {
      deltaMax = deltaTmp;
      threshold = (uint8)j;
    }
    if (deltaTmp < deltaMax) break;
  }
  return threshold;
}


#define LEFT_EDGE_END        15     // 左侧边缘区域结束列
#define MIDDLE_REGION_START  70     // 中间敏感区域起始列
#define MIDDLE_REGION_END    75     // 中间敏感区域结束列
#define RIGHT_REGION_START   80     // 右侧边缘区域起始列

void Get01change_dajin(void) {
    // 计算动态阈值（带阳光算法）
    ImageStatus.Threshold = Threshold_deal(
        Image_Use[0],
        LCDW,
        LCDH,
        ImageStatus.Threshold_detach
    );

    // 阈值下限保护
    if (ImageStatus.Threshold < ImageStatus.Threshold_static) {
        ImageStatus.Threshold = ImageStatus.Threshold_static;
    }

    // 预计算调整阈值（防溢出）
    const int adjusted_threshold = (ImageStatus.Threshold >= 10) ?
                                  (ImageStatus.Threshold + 10) : 0;

    // 预生成列阈值表
    uint8_t col_threshold[LCDW];
    for (int j = 0; j < LCDW; j++) {
        if (j <= LEFT_EDGE_END ||
           (j > MIDDLE_REGION_START && j <= MIDDLE_REGION_END) ||
            j > RIGHT_REGION_START) {
            col_threshold[j] = (uint8_t)adjusted_threshold;
        } else {
            col_threshold[j] = ImageStatus.Threshold;
        }
    }

    // 二值化处理
    for (int i = 0; i < LCDH; i++) {
        for (int j = 0; j < LCDW; j++) {
            const uint8_t thre = col_threshold[j];
            const uint8_t pixel = Image_Use[i][j];

            if (pixel > thre) {
                Pixle[i][j] = 1;          // 白
                display_image[i][j] = 255;
            } else {
                Pixle[i][j] = 0;          // 黑
                display_image[i][j] = 0;
            }
        }
    }
}

// 优化后的像素滤波函数（消除孤立黑点）
#define FILTER_START_ROW  10
#define FILTER_END_ROW    40
#define FILTER_START_COL  10
#define FILTER_END_COL    70
/**
 * 处理边界行（需要边界检查）
 */
static void Process_Border_Row(int row) {
    for (int col = FILTER_START_COL; col < FILTER_END_COL; ++col) {
        if (Pixle[row][col] != 0) continue;

        int sum = 0;
        if (row > 0) sum += Pixle[row-1][col];
        if (row < LCDH-1) sum += Pixle[row+1][col];
        if (col > 0) sum += Pixle[row][col-1];
        if (col < LCDW-1) sum += Pixle[row][col+1];

        if (sum >= 3) Pixle[row][col] = 1;
    }
}

/**
 * 处理边界列（需要边界检查）
 */
static void Process_Border_Col(int col) {
    for (int row = FILTER_START_ROW; row < FILTER_END_ROW; ++row) {
        if (Pixle[row][col] != 0) continue;

        int sum = 0;
        if (row > 0) sum += Pixle[row-1][col];
        if (row < LCDH-1) sum += Pixle[row+1][col];
        if (col > 0) sum += Pixle[row][col-1];
        if (col < LCDW-1) sum += Pixle[row][col+1];

        if (sum >= 3) Pixle[row][col] = 1;
    }
}

void Pixel_Filter() {
    // ================== 1.处理内部区域（无边界检查） ==================
    for (int row = FILTER_START_ROW + 1; row < FILTER_END_ROW - 1; ++row) {
        uint8_t *curr_row = Pixle[row];
        const uint8_t *upper_row = Pixle[row - 1];
        const uint8_t *lower_row = Pixle[row + 1];

        // 4列展开处理
        int col = FILTER_START_COL + 1;
        for (; col + 3 < FILTER_END_COL - 1; col += 4) {
            // 列1
            if (curr_row[col] == 0) {
                const int sum = upper_row[col] + lower_row[col] +
                              curr_row[col-1] + curr_row[col+1];
                curr_row[col] = (sum >= 3) ? 1 : 0;
            }
            // 列2
            if (curr_row[col+1] == 0) {
                const int sum = upper_row[col+1] + lower_row[col+1] +
                              curr_row[col] + curr_row[col+2];
                curr_row[col+1] = (sum >= 3) ? 1 : 0;
            }
            // 列3
            if (curr_row[col+2] == 0) {
                const int sum = upper_row[col+2] + lower_row[col+2] +
                              curr_row[col+1] + curr_row[col+3];
                curr_row[col+2] = (sum >= 3) ? 1 : 0;
            }
            // 列4
            if (curr_row[col+3] == 0) {
                const int sum = upper_row[col+3] + lower_row[col+3] +
                              curr_row[col+2] + curr_row[col+4];
                curr_row[col+3] = (sum >= 3) ? 1 : 0;
            }
        }

        // 处理剩余列
        for (; col < FILTER_END_COL - 1; ++col) {
            if (curr_row[col] == 0) {
                const int sum = upper_row[col] + lower_row[col] +
                              curr_row[col-1] + curr_row[col+1];
                curr_row[col] = (sum >= 3) ? 1 : 0;
            }
        }
    }

    // ================== 2.处理边界区域 ==================
    // 上边缘（row = FILTER_START_ROW）
    Process_Border_Row(FILTER_START_ROW);

    // 下边缘（row = FILTER_END_ROW - 1）
    Process_Border_Row(FILTER_END_ROW - 1);

    // 左边缘（col = FILTER_START_COL）
    Process_Border_Col(FILTER_START_COL);

    // 右边缘（col = FILTER_END_COL - 1）
    Process_Border_Col(FILTER_END_COL - 1);
}


static uint8 DrawLinesFirst(void) {
  PicTemp = Pixle[59];
  if (*(PicTemp + ImageSensorMid) == 0)                 //如果底边图像中点为黑，异常情况
  {
    for (Xsite = 0; Xsite < ImageSensorMid; Xsite++)    //找左右边线
    {
      if (*(PicTemp + ImageSensorMid - Xsite) != 0)     //一旦找到左或右赛道到中心距离，就break
        break;                                          //并且记录Xsite
      if (*(PicTemp + ImageSensorMid + Xsite) != 0)
        break;
    }

    if (*(PicTemp + ImageSensorMid - Xsite) != 0)       //赛道如果在左边的话
    {
      BottomBorderRight = ImageSensorMid - Xsite + 1;   // 59行右边线有啦
      for (Xsite = BottomBorderRight; Xsite > 0; Xsite--)  //开始找59行左边线
      {
        if (*(PicTemp + Xsite) == 0 &&
            *(PicTemp + Xsite - 1) == 0)                //连续两个黑点，滤波
        {
          BottomBorderLeft = Xsite;                     //左边线找到
          break;
        } else if (Xsite == 1) {
          BottomBorderLeft = 0;                         //搜索到最后了，看不到左边线，左边线认为是0
          break;
        }
      }
    } else if (*(PicTemp + ImageSensorMid + Xsite) != 0)  //赛道如果在右边的话
    {
      BottomBorderLeft = ImageSensorMid + Xsite - 1;    // 59行左边线有啦
      for (Xsite = BottomBorderLeft; Xsite < 79; Xsite++)  //开始找59行左边线
      {
        if (  *(PicTemp + Xsite) == 0
            &&*(PicTemp + Xsite + 1) == 0)              //连续两个黑点，滤波
        {
          BottomBorderRight = Xsite;                    //右边线找到
          break;
        } else if (Xsite == 78) {
          BottomBorderRight = 79;                       //搜索到最后了，看不到右边线，左边线认为是79
          break;
        }
      }
    }
  }
  else                                                //左边线中点是白的，比较正常的情况
  {
    for (Xsite = 79; Xsite >ImageSensorMid; Xsite--)   //一个点一个点地搜索右边线
    {
      if (  *(PicTemp + Xsite) == 1
          &&*(PicTemp + Xsite - 1) == 1)                //连续两个白点，滤波
      {
        BottomBorderRight = Xsite;                      //找到就记录
        break;
      } else if (Xsite == 40) {
        BottomBorderRight = 39;                         //找不到认为为中线
        break;
      }
    }
    for (Xsite = 0; Xsite < ImageSensorMid; Xsite++)    //一个点一个点地搜索左边线
    {
      if (  *(PicTemp + Xsite) == 1
          &&*(PicTemp + Xsite + 1) == 1)                //连续两个白点，滤波
      {
        BottomBorderLeft = Xsite;                       //找到就记录
        break;
      } else if (Xsite == 38) {
        BottomBorderLeft = 39;                           //找不到认为中间
        break;
      }
    }
  }
  BottomCenter =(BottomBorderLeft + BottomBorderRight) / 2;   // 59行中点直接取平均
  ImageDeal[59].LeftBorder = BottomBorderLeft;                //在数组里面记录一下信息，第一行特殊一点而已
  ImageDeal[59].RightBorder = BottomBorderRight;
  ImageDeal[59].Center = BottomCenter;                        //确定最底边
  ImageDeal[59].Wide = BottomBorderRight - BottomBorderLeft;  //存储宽度信息
  ImageDeal[59].IsLeftFind = 'T';
  ImageDeal[59].IsRightFind = 'T';
  for (Ysite = 58; Ysite > 54; Ysite--)                       //由中间向两边确定底边五行
  {
    PicTemp = Pixle[Ysite];
    for (Xsite = 79; Xsite > ImageDeal[Ysite + 1].Center;Xsite--)                                             //和前面一样的搜索
    {
      if (*(PicTemp + Xsite) == 1 && *(PicTemp + Xsite - 1) == 1) {
        ImageDeal[Ysite].RightBorder = Xsite;
        break;
      } else if (Xsite == (ImageDeal[Ysite + 1].Center+1)) {
        ImageDeal[Ysite].RightBorder = ImageDeal[Ysite + 1].Center;
        break;
      }
    }
    for (Xsite = 0; Xsite < ImageDeal[Ysite + 1].Center;Xsite++)                                             //和前面一样的搜索
    {
      if (*(PicTemp + Xsite) == 1 && *(PicTemp + Xsite + 1) == 1) {
        ImageDeal[Ysite].LeftBorder = Xsite;
        break;
      } else if (Xsite == (ImageDeal[Ysite + 1].Center-1)) {
        ImageDeal[Ysite].LeftBorder = ImageDeal[Ysite + 1].Center;
        break;
      }
    }
    ImageDeal[Ysite].IsLeftFind = 'T';                        //这些信息存储到数组里
    ImageDeal[Ysite].IsRightFind = 'T';
    ImageDeal[Ysite].Center =
        (ImageDeal[Ysite].RightBorder + ImageDeal[Ysite].LeftBorder) /2; //存储中点
    ImageDeal[Ysite].Wide =
        ImageDeal[Ysite].RightBorder - ImageDeal[Ysite].LeftBorder;      //存储宽度
  }
  return 'T';//函数返回 'T'，表示成功提取了底部五行的赛道边界信息。
}                                                             //最基本的要求，最近的五行首先不会受到干扰，这需要在安装的时候调整摄像头的视角

void GetJumpPointFromDet(uint8* p,uint8 type,int L,int H,JumpPointtypedef* Q)  //第一个参数是要查找的数组（80个点）
                                                                               //第二个扫左边线还是扫右边线
{                                                                              //三四是开始和结束点
  int i = 0;
  if (type == 'L')                              //扫描左边线
  {
    for (i = H; i >= L; i--) {
      if (*(p + i) == 1 && *(p + i - 1) != 1)   //由黑变白
      {
        Q->point = i;                           //记录左边线
        Q->type = 'T';                          //正确跳变
        break;
      } else if (i == (L + 1))                  //若果扫到最后也没找到
      {
        if (*(p + (L + H) / 2) != 0)            //如果中间是白的
        {
          Q->point = (L + H) / 2;               //认为左边线是中点
          Q->type = 'W';                        //非正确跳变且中间为白，认为没有边
          break;
        } else                                  //非正确跳变且中间为黑
        {
          Q->point = H;                         //如果中间是黑的
          Q->type = 'H';                        //左边线直接最大值，认为是大跳变
          break;
        }
      }
    }
  } else if (type == 'R')                       //扫描右边线
  {
    for (i = L; i <= H; i++)                    //从右往左扫
    {
      if (*(p + i) == 1 && *(p + i + 1) != 1)   //找由黑到白的跳变
      {
        Q->point = i;                           //记录
        Q->type = 'T';
        break;
      } else if (i == (H - 1))                  //若果扫到最后也没找到
      {
        if (*(p + (L + H) / 2) != 0)            //如果中间是白的
        {
          Q->point = (L + H) / 2;               //右边线是中点
          Q->type = 'W';
          break;
        } else                                  //如果中点是黑的
        {
          Q->point = L;                         //左边线直接最大值
          Q->type = 'H';
          break;
        }
      }
    }
  }
}

/*边线追逐大致得到全部边线*/
static void DrawLinesProcess(void)  //////不用更改
{
  uint8 L_Found_T = 'F';  //确定无边斜率的基准有边行是否被找到的标志
  uint8 Get_L_line = 'F';  //找到这一帧图像的基准左斜率
  uint8 R_Found_T = 'F';  //确定无边斜率的基准有边行是否被找到的标志
  uint8 Get_R_line = 'F';  //找到这一帧图像的基准右斜率
  float D_L = 0;           //延长线左边线斜率
  float D_R = 0;           //延长线右边线斜率
  int ytemp_W_L;           //记住首次左丢边行
  int ytemp_W_R;           //记住首次右丢边行
  ExtenRFlag = 0;          //标志位清0
  ExtenLFlag = 0;
   ImageStatus.Left_Line = 0;
   ImageStatus.WhiteLine = 0;
   ImageStatus.Right_Line = 0;
  for (Ysite = 54 ; Ysite > ImageStatus.OFFLine; Ysite--)            //前5行处理过了，下面从55行到（设定的不处理的行OFFLine）
  {                        //太远的图像不稳定，OFFLine以后的不处理
    PicTemp = Pixle[Ysite];
    JumpPointtypedef JumpPoint[2];                                          // 0左1右
    if (ImageStatus.Road_type != Cross_ture
           /* &&SystemData.SpeedData.Length*OX>500*/) {
      IntervalLow =ImageDeal[Ysite + 1].RightBorder -ImageScanInterval;             //从上一行右边线-Interval的点开始（确定扫描开始点）
      IntervalHigh =ImageDeal[Ysite + 1].RightBorder + ImageScanInterval;           //到上一行右边线+Interval的点结束（确定扫描结束点）
    } else {
      IntervalLow =ImageDeal[Ysite + 1].RightBorder -ImageScanInterval_Cross;       //从上一行右边线-Interval_Cross的点开始（确定扫描开始点）
      IntervalHigh = ImageDeal[Ysite + 1].RightBorder + ImageScanInterval_Cross;    //到上一行右边线+Interval_Cross的点开始（确定扫描开始点）
    }

    LimitL(IntervalLow);   //确定左扫描区间并进行限制
    LimitH(IntervalHigh);  //确定右扫描区间并进行限制
    GetJumpPointFromDet(PicTemp, 'R', IntervalLow, IntervalHigh,&JumpPoint[1]);     //扫右边线

    IntervalLow =ImageDeal[Ysite + 1].LeftBorder -ImageScanInterval;                //从上一行左边线-5的点开始（确定扫描开始点）
    IntervalHigh =ImageDeal[Ysite + 1].LeftBorder +ImageScanInterval;               //到上一行左边线+5的点结束（确定扫描结束点）

    LimitL(IntervalLow);   //确定左扫描区间并进行限制
    LimitH(IntervalHigh);  //确定右扫描区间并进行限制
    GetJumpPointFromDet(PicTemp, 'L', IntervalLow, IntervalHigh,&JumpPoint[0]);

    if (JumpPoint[0].type =='W')                                                    //如果本行左边线不正常跳变，即这10个点都是白的
    {
      ImageDeal[Ysite].LeftBorder =ImageDeal[Ysite + 1].LeftBorder;                 //本行左边线用上一行的数值
    } else                                                                          //左边线正常
    {
      ImageDeal[Ysite].LeftBorder = JumpPoint[0].point;                             //记录下来啦
    }

    if (JumpPoint[1].type == 'W')                                                   //如果本行右边线不正常跳变
    {
      ImageDeal[Ysite].RightBorder =ImageDeal[Ysite + 1].RightBorder;               //本行右边线用上一行的数值
    } else                                                                          //右边线正常
    {
      ImageDeal[Ysite].RightBorder = JumpPoint[1].point;                            //记录下来啦
    }

    ImageDeal[Ysite].IsLeftFind =JumpPoint[0].type;                                 //记录本行是否找到边线，即边线类型
    ImageDeal[Ysite].IsRightFind = JumpPoint[1].type;

    //重新确定那些大跳变的边缘
    if (( ImageDeal[Ysite].IsLeftFind == 'H'
         ||ImageDeal[Ysite].IsRightFind == 'H')) {
      if (ImageDeal[Ysite].IsLeftFind == 'H')                                   //如果左边线大跳变
        for (Xsite = (ImageDeal[Ysite].LeftBorder + 1);
             Xsite <= (ImageDeal[Ysite].RightBorder - 1);
             Xsite++)                                                           //左右边线之间重新扫描
        {
          if ((*(PicTemp + Xsite) == 0) && (*(PicTemp + Xsite + 1) != 0)) {
            ImageDeal[Ysite].LeftBorder =Xsite;                                 //如果上一行左边线的右边有黑白跳变则为绝对边线直接取出
            ImageDeal[Ysite].IsLeftFind = 'T';
            break;
          } else if (*(PicTemp + Xsite) != 0)                                   //一旦出现白点则直接跳出
            break;
          else if (Xsite ==(ImageDeal[Ysite].RightBorder - 1))
          {
             ImageDeal[Ysite].IsLeftFind = 'T';
            break;
          }
        }
      if ((ImageDeal[Ysite].RightBorder - ImageDeal[Ysite].LeftBorder) <=
          7)                              //图像宽度限定
      {
        ImageStatus.OFFLine = Ysite + 1;  //如果这行比7小了后面直接不要了
        break;
      }
      if (ImageDeal[Ysite].IsRightFind == 'H')
        for (Xsite = (ImageDeal[Ysite].RightBorder - 1);
             Xsite >= (ImageDeal[Ysite].LeftBorder + 1); Xsite--) {
          if ((*(PicTemp + Xsite) == 0) && (*(PicTemp + Xsite - 1) != 0)) {
            ImageDeal[Ysite].RightBorder =
                Xsite;                    //如果右边线的左边还有黑白跳变则为绝对边线直接取出
            ImageDeal[Ysite].IsRightFind = 'T';
            break;
          } else if (*(PicTemp + Xsite) != 0)
            break;
          else if (Xsite == (ImageDeal[Ysite].LeftBorder + 1))
          {
            ImageDeal[Ysite].RightBorder = Xsite;
            ImageDeal[Ysite].IsRightFind = 'T';
            break;
          }
        }
    }

 /***********重新确定无边行************/
    int ysite = 0;
    uint8 L_found_point = 0;
    uint8 R_found_point = 0;


    if(  ImageStatus.Road_type != Ramp)
    {
    if (    ImageDeal[Ysite].IsRightFind == 'W'
          &&Ysite > 10
          &&Ysite < 50
          &&ImageStatus.Road_type!=Barn_in
          )                     //最早出现的无边行
    {
      if (Get_R_line == 'F')    //这一帧图像没有跑过这个找基准线的代码段才运行
      {
        Get_R_line = 'T';       //找了  一帧图像只跑一次 置为T
        ytemp_W_R = Ysite + 2;
        for (ysite = Ysite + 1; ysite < Ysite + 15; ysite++) {
          if (ImageDeal[ysite].IsRightFind =='T')  //往无边行下面搜索  一般都是有边的
            R_found_point++;
        }
        if (R_found_point >8)                      //找到基准斜率边  做延长线重新确定无边   当有边的点数大于8
        {
          D_R = ((float)(ImageDeal[Ysite + R_found_point].RightBorder - ImageDeal[Ysite + 3].RightBorder)) /((float)(R_found_point - 3));
                                                  //求下面这些点连起来的斜率
                                                  //好给无边行做延长线左个基准
          if (D_R > 0) {
            R_Found_T ='T';                       //如果斜率大于0  那么找到了这个基准行  因为梯形畸变
                                                  //所以一般情况都是斜率大于0  小于0的情况也不用延长 没必要
          } else {
            R_Found_T = 'F';                      //没有找到这个基准行
            if (D_R < 0)
              ExtenRFlag = 'F';                   //这个标志位用于十字角点补线  防止图像误补用的
          }
        }
      }
      if (R_Found_T == 'T')
        ImageDeal[Ysite].RightBorder =ImageDeal[ytemp_W_R].RightBorder -D_R * (ytemp_W_R - Ysite);  //如果找到了 那么以基准行做延长线

      LimitL(ImageDeal[Ysite].RightBorder);  //限幅
      LimitH(ImageDeal[Ysite].RightBorder);  //限幅
    }

    if (ImageDeal[Ysite].IsLeftFind == 'W' && Ysite > 10 && Ysite < 50 &&
        ImageStatus.Road_type != Barn_in)    //下面同理  左边界
    {
      if (Get_L_line == 'F') {
        Get_L_line = 'T';
        ytemp_W_L = Ysite + 2;
        for (ysite = Ysite + 1; ysite < Ysite + 15; ysite++) {
          if (ImageDeal[ysite].IsLeftFind == 'T')
            L_found_point++;
        }
        if (L_found_point > 8)              //找到基准斜率边  做延长线重新确定无边
        {
          D_L = ((float)(ImageDeal[Ysite + 3].LeftBorder -ImageDeal[Ysite + L_found_point].LeftBorder)) /((float)(L_found_point - 3));
          if (D_L > 0) {
            L_Found_T = 'T';

          } else {
            L_Found_T = 'F';
            if (D_L < 0)
              ExtenLFlag = 'F';
          }
        }
      }

      if (L_Found_T == 'T')
        ImageDeal[Ysite].LeftBorder =ImageDeal[ytemp_W_L].LeftBorder + D_L * (ytemp_W_L - Ysite);

      LimitL(ImageDeal[Ysite].LeftBorder);  //限幅
      LimitH(ImageDeal[Ysite].LeftBorder);  //限幅
    }
}
    if (ImageDeal[Ysite].IsLeftFind == 'W'&&ImageDeal[Ysite].IsRightFind == 'W')
         {
             ImageStatus.WhiteLine++;  //要是左右都无边，丢边数+1
         }
        if (ImageDeal[Ysite].IsLeftFind == 'W'&&Ysite<55)
        {
             ImageStatus.Left_Line++;
        }
        if (ImageDeal[Ysite].IsRightFind == 'W'&&Ysite<55)
        {
             ImageStatus.Right_Line++;
        }


      LimitL(ImageDeal[Ysite].LeftBorder);   //限幅
      LimitH(ImageDeal[Ysite].LeftBorder);   //限幅
      LimitL(ImageDeal[Ysite].RightBorder);  //限幅
      LimitH(ImageDeal[Ysite].RightBorder);  //限幅

      ImageDeal[Ysite].Wide =ImageDeal[Ysite].RightBorder - ImageDeal[Ysite].LeftBorder;
      ImageDeal[Ysite].Center =(ImageDeal[Ysite].RightBorder + ImageDeal[Ysite].LeftBorder) / 2;

    if (ImageDeal[Ysite].Wide <= 7)         //重新确定可视距离
    {
      ImageStatus.OFFLine = Ysite + 1;
      break;
    }

    else if (  ImageDeal[Ysite].RightBorder <= 10
             ||ImageDeal[Ysite].LeftBorder >= 70) {
              ImageStatus.OFFLine = Ysite + 1;
              break;
    }                                        //当图像宽度小于0或者左右边达到一定的限制时，则终止巡边
  }


  return;
}
/*****************直线判断******************/
float Straight_Judge(uint8 dir, uint8 start, uint8 end)
{
    int i;
    float S = 0, Sum = 0, Err = 0, k = 0;
    switch (dir)
    {
    case 1:k = (float)(ImageDeal[start].LeftBorder - ImageDeal[end].LeftBorder) / (start - end);
        for (i = 0; i < end - start; i++)
        {
            Err = (ImageDeal[start].LeftBorder + k * i - ImageDeal[i + start].LeftBorder) * (ImageDeal[start].LeftBorder + k * i - ImageDeal[i + start].LeftBorder);
            Sum += Err;
        }
        S = Sum / (end - start);
        break;
    case 2:k = (float)(ImageDeal[start].RightBorder - ImageDeal[end].RightBorder) / (start - end);
        for (i = 0; i < end - start; i++)
        {
            Err = (ImageDeal[start].RightBorder + k * i - ImageDeal[i + start].RightBorder) * (ImageDeal[start].RightBorder + k * i - ImageDeal[i + start].RightBorder);
            Sum += Err;
        }
        S = Sum / (end - start);
        break;
    }
    return S;
}

/*上交大左右手法则扫线，作为处理圆环等判断元素的第二依据*/
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Bottom_Line_OTSU
//  @brief          获取底层左右边线
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        传入的图像数组
//  @param          Row                                     图像的Ysite
//  @param          Col                                     图像的Xsite
//  @return         Bottonline                              底边行选择
//  @time           2022年10月9日
//  @Author
//  Sample usage:   Search_Bottom_Line_OTSU(imageInput, Row, Col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Bottom_Line_OTSU(uint8 imageInput[LCDH][LCDW], uint8 Row, uint8 Col, uint8 Bottonline)
{

    //寻找左边边界
    for (int Xsite = Col / 2-2; Xsite > 1; Xsite--)
    {
        if (imageInput[Bottonline][Xsite] == 1 && imageInput[Bottonline][Xsite - 1] == 0)
        {
            ImageDeal[Bottonline].LeftBoundary = Xsite;//获取底边左边线
            break;
        }
    }
    for (int Xsite = Col / 2+2; Xsite < LCDW-1; Xsite++)
    {
        if (imageInput[Bottonline][Xsite] == 1 && imageInput[Bottonline][Xsite + 1] == 0)
        {
            ImageDeal[Bottonline].RightBoundary = Xsite;//获取底边右边线
            break;
        }
    }

}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Left_and_Right_Lines
//  @brief          通过sobel提取左右边线
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        传入的图像数组
//  @param          Row                                     图像的Ysite
//  @param          Col                                     图像的Xsite
//  @param          Bottonline                              底边行选择
//  @return         无
//  @time           2022年10月7日
//  @Author
//  Sample usage:   Search_Left_and_Right_Lines(imageInput, Row, Col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Left_and_Right_Lines(uint8 imageInput[LCDH][LCDW], uint8 Row, uint8 Col, uint8 Bottonline)
{
    //定义小人的当前行走状态位置为 上 左 下 右 一次要求 上：左边为黑色 左：上边为褐色 下：右边为色  右：下面有黑色
/*  前进方向定义：
                *   0
                * 3   1
                *   2
*/
/*寻左线坐标规则*/
    uint8 Left_Rule[2][8] = {
                                  {0,-1,1,0,0,1,-1,0 },//{0,-1},{1,0},{0,1},{-1,0},  (x,y )
                                  {-1,-1,1,-1,1,1,-1,1} //{-1,-1},{1,-1},{1,1},{-1,1}
    };
    /*寻右线坐标规则*/
    int Right_Rule[2][8] = {
                              {0,-1,1,0,0,1,-1,0 },//{0,-1},{1,0},{0,1},{-1,0},
                              {1,-1,1,1,-1,1,-1,-1} //{1,-1},{1,1},{-1,1},{-1,-1}
    };
      int num=0;
    uint8 Left_Ysite = Bottonline;
    uint8 Left_Xsite = (uint8)ImageDeal[Bottonline].LeftBoundary;
    uint8 Left_Rirection = 0;//左边方向
    uint8 Pixel_Left_Ysite = Bottonline;
    uint8 Pixel_Left_Xsite = 0;

    uint8 Right_Ysite = Bottonline;
    uint8 Right_Xsite = (uint8)ImageDeal[Bottonline].RightBoundary;
    uint8 Right_Rirection = 0;//右边方向
    uint8 Pixel_Right_Ysite = Bottonline;
    uint8 Pixel_Right_Xsite = 0;
    uint8 Ysite = Bottonline;
    ImageStatus.OFFLineBoundary = 5;
    while (1)
    {
            num++;
            if(num>400)
            {
                 ImageStatus.OFFLineBoundary = Ysite;
                break;
            }
        if (Ysite >= Pixel_Left_Ysite && Ysite >= Pixel_Right_Ysite)
        {
            if (Ysite < ImageStatus.OFFLineBoundary)
            {
                ImageStatus.OFFLineBoundary = Ysite;
                break;
            }
            else
            {
                Ysite--;
            }
        }
        /*********左边巡线*******/
        if ((Pixel_Left_Ysite > Ysite) || Ysite == ImageStatus.OFFLineBoundary)//右边扫线
        {
            /*计算前方坐标*/
            Pixel_Left_Ysite = Left_Ysite + Left_Rule[0][2 * Left_Rirection + 1];
            Pixel_Left_Xsite = Left_Xsite + Left_Rule[0][2 * Left_Rirection];

            if (imageInput[Pixel_Left_Ysite][Pixel_Left_Xsite] == 0)//前方是黑色
            {
                //顺时针旋转90
                if (Left_Rirection == 3)
                    Left_Rirection = 0;
                else
                    Left_Rirection++;
            }
            else//前方是白色
            {
                /*计算左前方坐标*/
                Pixel_Left_Ysite = Left_Ysite + Left_Rule[1][2 * Left_Rirection + 1];
                Pixel_Left_Xsite = Left_Xsite + Left_Rule[1][2 * Left_Rirection];

                if (imageInput[Pixel_Left_Ysite][Pixel_Left_Xsite] == 0)//左前方为黑色
                {
                    //方向不变  Left_Rirection
                    Left_Ysite = Left_Ysite + Left_Rule[0][2 * Left_Rirection + 1];
                    Left_Xsite = Left_Xsite + Left_Rule[0][2 * Left_Rirection];
                    if (ImageDeal[Left_Ysite].LeftBoundary_First == 0){
                        ImageDeal[Left_Ysite].LeftBoundary_First = Left_Xsite;
                        ImageDeal[Left_Ysite].LeftBoundary = Left_Xsite;
                    }
                }
                else//左前方为白色
                {
                    // 方向发生改变 Left_Rirection  逆时针90度
                    Left_Ysite = Left_Ysite + Left_Rule[1][2 * Left_Rirection + 1];
                    Left_Xsite = Left_Xsite + Left_Rule[1][2 * Left_Rirection];
                    if (ImageDeal[Left_Ysite].LeftBoundary_First == 0 )
                        ImageDeal[Left_Ysite].LeftBoundary_First = Left_Xsite;
                    ImageDeal[Left_Ysite].LeftBoundary = Left_Xsite;
                    if (Left_Rirection == 0)
                        Left_Rirection = 3;
                    else
                        Left_Rirection--;
                }

            }
        }
        /*********时光-贺兰一号开源*******/
        /*********右边巡线*******/
        if ((Pixel_Right_Ysite > Ysite) || Ysite == ImageStatus.OFFLineBoundary)//右边扫线
        {
            /*计算前方坐标*/
            Pixel_Right_Ysite = Right_Ysite + Right_Rule[0][2 * Right_Rirection + 1];
            Pixel_Right_Xsite = Right_Xsite + Right_Rule[0][2 * Right_Rirection];

            if (imageInput[Pixel_Right_Ysite][Pixel_Right_Xsite] == 0)//前方是黑色
            {
                //逆时针旋转90
                if (Right_Rirection == 0)
                    Right_Rirection = 3;
                else
                    Right_Rirection--;
            }
            else//前方是白色
            {
                /*计算右前方坐标*/
                Pixel_Right_Ysite = Right_Ysite + Right_Rule[1][2 * Right_Rirection + 1];
                Pixel_Right_Xsite = Right_Xsite + Right_Rule[1][2 * Right_Rirection];

                if (imageInput[Pixel_Right_Ysite][Pixel_Right_Xsite] == 0)//左前方为黑色
                {
                    //方向不变  Right_Rirection
                    Right_Ysite = Right_Ysite + Right_Rule[0][2 * Right_Rirection + 1];
                    Right_Xsite = Right_Xsite + Right_Rule[0][2 * Right_Rirection];
                    if (ImageDeal[Right_Ysite].RightBoundary_First == 79 )
                        ImageDeal[Right_Ysite].RightBoundary_First = Right_Xsite;
                    ImageDeal[Right_Ysite].RightBoundary = Right_Xsite;
                }
                else//左前方为白色
                {
                    // 方向发生改变 Right_Rirection  逆时针90度
                    Right_Ysite = Right_Ysite + Right_Rule[1][2 * Right_Rirection + 1];
                    Right_Xsite = Right_Xsite + Right_Rule[1][2 * Right_Rirection];
                    if (ImageDeal[Right_Ysite].RightBoundary_First == 79)
                        ImageDeal[Right_Ysite].RightBoundary_First = Right_Xsite;
                    ImageDeal[Right_Ysite].RightBoundary = Right_Xsite;
                    if (Right_Rirection == 3)
                        Right_Rirection = 0;
                    else
                        Right_Rirection++;
                }

            }
        }

        if (abs(Pixel_Right_Xsite - Pixel_Left_Xsite) < 3)//Ysite<80是为了放在底部是斑马线扫描结束  3 && Ysite < 30
        {

            ImageStatus.OFFLineBoundary = Ysite;
            break;
        }

    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Border_OTSU
//  @brief          通过OTSU获取边线 和信息
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        传入的图像数组
//  @param          Row                                     图像的Ysite
//  @param          Col                                     图像的Xsite
//  @param          Bottonline                              底边行选择
//  @return         无
//  @time           2022年10月7日
//  @Author
//  Sample usage:   Search_Border_OTSU(mt9v03x_image, IMAGE_ROW, IMAGE_COL, IMAGE_ROW-8);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Border_OTSU(uint8 imageInput[LCDH][LCDW], uint8 Row, uint8 Col, uint8 Bottonline)
{
    ImageStatus.WhiteLine_L = 0;
    ImageStatus.WhiteLine_R = 0;
    //ImageStatus.OFFLine = 1;
    /*封上下边界处理*/
    for (int Xsite = 0; Xsite < LCDW; Xsite++)
    {
        imageInput[0][Xsite] = 0;
        imageInput[Bottonline + 1][Xsite] = 0;
    }
    /*封左右边界处理*/
    for (int Ysite = 0; Ysite < LCDH; Ysite++)
    {
            ImageDeal[Ysite].LeftBoundary_First = 0;
            ImageDeal[Ysite].RightBoundary_First = 79;

            imageInput[Ysite][0] = 0;
            imageInput[Ysite][LCDW - 1] = 0;
    }
    /********获取底部边线*********/
    Search_Bottom_Line_OTSU(imageInput, Row, Col, Bottonline);
    /********获取左右边线*********/
    Search_Left_and_Right_Lines(imageInput, Row, Col, Bottonline);



    for (int Ysite = Bottonline; Ysite > ImageStatus.OFFLineBoundary + 1; Ysite--)
    {
        if (ImageDeal[Ysite].LeftBoundary < 3)
        {
            ImageStatus.WhiteLine_L++;
        }
        if (ImageDeal[Ysite].RightBoundary > LCDW - 3)
        {
            ImageStatus.WhiteLine_R++;
        }
    }
}
//延长线绘制，理论上来说是很准确的
static void DrawExtensionLine(void)        //绘制延长线并重新确定中线 ，把补线补成斜线
{
  if ((
        ImageStatus.Road_type != Barn_in
        &&ImageStatus.Road_type != Ramp)
//        &&ImageStatus.pansancha_Lenth* OX==0
        &&ImageStatus.Road_type !=LeftCirque
        &&ImageStatus.Road_type !=RightCirque
        )                                  // g5.22  6.22调试注释  记得改回来
  {
    if (ImageStatus.WhiteLine >= ImageStatus.TowPoint_True - 15)
      TFSite = 55;
//    if (ImageStatus.CirqueOff == 'T' && ImageStatus.Road_type == LeftCirque)
//      TFSite = 55;
    if (ExtenLFlag != 'F')
      for (Ysite = 54; Ysite >= (ImageStatus.OFFLine + 4);
           Ysite--)                       //从第五行开始网上扫扫到顶边下面两行   多段补线
                                          //不仅仅只有一段
      {
        PicTemp = Pixle[Ysite];           //存当前行
        if (ImageDeal[Ysite].IsLeftFind =='W')                          //如果本行左边界没扫到但扫到的是白色，说明本行没有左边界点
        {
          //**************************************************//**************************************************
          if (ImageDeal[Ysite + 1].LeftBorder >= 70)                    //如果左边界实在是太右边
          {
            ImageStatus.OFFLine = Ysite + 1;
            break;                        //直接跳出（极端情况）
          }
          //************************************************//*************************************************

          while (Ysite >= (ImageStatus.OFFLine + 4))                    //此时还没扫到顶边
          {
            Ysite--;                      //继续往上扫
            if (  ImageDeal[Ysite].IsLeftFind == 'T'
                &&ImageDeal[Ysite - 1].IsLeftFind == 'T'
                &&ImageDeal[Ysite - 2].IsLeftFind == 'T'
                &&ImageDeal[Ysite - 2].LeftBorder > 0
                &&ImageDeal[Ysite - 2].LeftBorder <70
                )                                                       //如果扫到本行出现了并且本行以上连续三行都有左边界点（左边界在空白上方）
            {
              FTSite = Ysite - 2;          //把本行上面的第二行存入FTsite
              break;
            }
          }

          DetL =
              ((float)(ImageDeal[FTSite].LeftBorder -
                       ImageDeal[TFSite].LeftBorder)) /
              ((float)(FTSite - TFSite));  //左边界的斜率：列的坐标差/行的坐标差
          if (FTSite > ImageStatus.OFFLine)
            for (
                ytemp = TFSite; ytemp >= FTSite; ytemp--)               //从第一次扫到的左边界的下面第二行的坐标开始往上扫直到空白上方的左边界的行坐标值
            {
              ImageDeal[ytemp].LeftBorder =
                  (int)(DetL * ((float)(ytemp - TFSite))) +
                  ImageDeal[TFSite]
                      .LeftBorder;                                      //将这期间的空白处补线（补斜线），目的是方便图像处理
            }
        } else
          TFSite = Ysite + 2;                                           //如果扫到了本行的左边界，该行存在这里面，（算斜率）
      }

    if (ImageStatus.WhiteLine >= ImageStatus.TowPoint_True - 15)
      TFSite = 55;
    // g5.22
    if (ImageStatus.CirqueOff == 'T' && ImageStatus.Road_type == RightCirque)
      TFSite = 55;
    if (ExtenRFlag != 'F')
      for (Ysite = 54; Ysite >= (ImageStatus.OFFLine + 4);
           Ysite--)               //从第五行开始网上扫扫到顶边下面两行
      {
        PicTemp = Pixle[Ysite];  //存当前行

        if (ImageDeal[Ysite].IsRightFind =='W')                       //如果本行右边界没扫到但扫到的是白色，说明本行没有右边界点，但是处于赛道内的
        {
          if (ImageDeal[Ysite + 1].RightBorder <= 10)                 //如果右边界实在是太左边
          {
            ImageStatus.OFFLine =Ysite + 1;                           //直接跳出，说明这种情况赛道就尼玛离谱
            break;
          }
          while (Ysite >= (ImageStatus.OFFLine + 4))                  //此时还没扫到顶边下面两行
          {
            Ysite--;
            if (  ImageDeal[Ysite].IsRightFind == 'T'
                &&ImageDeal[Ysite - 1].IsRightFind == 'T'
                &&ImageDeal[Ysite - 2].IsRightFind == 'T'
                &&ImageDeal[Ysite - 2].RightBorder < 70
                &&ImageDeal[Ysite - 2].RightBorder > 10
                )                                                      //如果扫到本行出现了并且本行以上连续三行都有左边界点（左边界在空白上方）
            {
              FTSite = Ysite - 2;                                      // 把本行上面的第二行存入FTsite
              break;
            }
          }

          DetR =((float)(ImageDeal[FTSite].RightBorder -ImageDeal[TFSite].RightBorder)) /((float)(FTSite - TFSite));         //右边界的斜率：列的坐标差/行的坐标差
          if (FTSite > ImageStatus.OFFLine)
            for (ytemp = TFSite; ytemp >= FTSite;ytemp--)              //从第一次扫到的右边界的下面第二行的坐标开始往上扫直到空白上方的右边界的行坐标值
            {
              ImageDeal[ytemp].RightBorder =(int)(DetR * ((float)(ytemp - TFSite))) +ImageDeal[TFSite].RightBorder;          //将这期间的空白处补线（补斜线），目的是方便图像处理
            }
        } else
          TFSite =Ysite +2;                                           //如果本行的右边界找到了，则把该行下面第二行坐标送个TFsite
      }
  }
  for (Ysite = 59; Ysite >= ImageStatus.OFFLine; Ysite--) {
    ImageDeal[Ysite].Center =(ImageDeal[Ysite].LeftBorder + ImageDeal[Ysite].RightBorder) /2;                                //扫描结束，把这一块经优化之后的中间值存入
    ImageDeal[Ysite].Wide =-ImageDeal[Ysite].LeftBorder +ImageDeal[Ysite].RightBorder;                                       //把优化之后的宽度存入
  }
}
/**
 * @brief 赛道中线修复与平滑滤波处理（效率优化版）
 * @note 集成边界检测、参考行搜索、中线插值修复、滑动平均滤波功能
 */
static void RouteFilter(void)
{
    // ===================== 编译时常量优化 =====================
    #define START_ROW           (58 > LCDH-1 ? LCDH-1 : 58)  // 安全起始行
    #define MAX_FILTER_ROW      45
    #define SEARCH_OFFSET       (-6)
    #define SMOOTHING_FACTOR    3
    #define MIN_VALID_ROWS      2
    #define SAFETY_MARGIN       5

    // ===================== 运行时预计算 =====================
    const int stopRow = (ImageStatus.OFFLine > SAFETY_MARGIN) ?
                       (ImageStatus.OFFLine + SAFETY_MARGIN) : 0;
    const int realStartRow = (START_ROW > stopRow) ? START_ROW : stopRow;

    // ===================== 主处理循环优化 =====================
    for (int row = realStartRow; row >= stopRow; row--)
    {
        /*---------------- 边界丢失检测优化 ----------------*/
        if (row <= MAX_FILTER_ROW && row > 0) // 隐含row>=1
        {

            if (ImageDeal[row].IsLeftFind == 'W' && ImageDeal[row].IsRightFind == 'W' &&
                    ImageDeal[row-1].IsLeftFind    == 'W' && ImageDeal[row-1].IsRightFind    == 'W')
            {
                /*-------- 参考行搜索优化 --------*/
                int refRow = -1;
                const int minSearchRow = (row + SEARCH_OFFSET > 0) ?
                                        row + SEARCH_OFFSET : 0;

                // 倒序搜索可提前退出
                for (int searchRow = row; searchRow >= minSearchRow; searchRow--)
                {
                    if (ImageDeal[searchRow].IsLeftFind == 'T' &&
                            ImageDeal[searchRow].IsRightFind == 'T')
                    {
                        refRow = searchRow;
                        break; // 找到即退出
                    }
                }

                /*-------- 中线插值修复优化 --------*/
                if (refRow != -1 && (row - refRow) >= MIN_VALID_ROWS)
                {
                    // 使用参考行下方有效数据计算斜率
                    const int baseRow = refRow + 1;
                    if (baseRow < LCDH)
                    {
                        const int centerBase = ImageDeal[baseRow].Center;
                        const int centerRef = ImageDeal[refRow].Center;
                        const float slope = (float)(centerRef - centerBase) /
                                          (refRow - baseRow); // 正向斜率

                        // 单次计算避免重复访问
                        for (int y = row; y > refRow; y--)
                        {
                            ImageDeal[y].Center = (int)(centerBase +
                                slope * (y - baseRow));
                        }
                    }
                }
            }
        }

        /*---------------- 滑动平均滤波优化 ----------------*/
        if (row > 0) // 循环范围保证row<LCDH
        {
            ImageDeal[row].Center =
                (ImageDeal[row-1].Center + 2*ImageDeal[row].Center) /
                SMOOTHING_FACTOR;
        }
    }

    // ===================== 清理宏定义 =====================
    #undef START_ROW
    #undef MAX_FILTER_ROW
    #undef SEARCH_OFFSET
    #undef SMOOTHING_FACTOR
    #undef MIN_VALID_ROWS
    #undef SAFETY_MARGIN
}
/********************* 误差加权计算模块（优化版） **********************
 * 优化点: 循环展开减少分支、预计算循环边界、指针访问优化内存操作
 **********************************************************************/

void GetDet()
{
    // ===================== 编译时常量优化 =====================
    #define MIN_SPEED_GAIN      (-1.0f)
    #define MAX_SPEED_GAIN      3.0f
    #define CIRCLE_TOW_POINT    30
    #define CROSS_TOW_POINT     22
    #define MAX_TOW_POINT       49
    #define WEIGHT_BASE         1.0f
    #define PARTIAL_OFFSET      5
    #define SAFE_RANGE_Y(y)     ((y) >= 0 && (y) < LCDH) // 安全访问宏
    #define MAX(a, b) ((a) > (b) ? (a) : (b))
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
    #define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

    // =================== 1.速度增益计算优化 ===================
    /* 使用整数运算优化浮点计算（根据硬件支持调整） */
    const float speedDiff = SystemData.SpeedData.nowspeed - SystemData.SpeedData.MinSpeed;
    float speedGain = speedDiff * 0.2f + 0.5f; // 保留浮点避免精度损失
    speedGain = CLAMP(speedGain, MIN_SPEED_GAIN, MAX_SPEED_GAIN); // 封装限幅宏

    // =================== 2.前瞻点决策优化 =====================
    int towPoint = MAX_TOW_POINT;
    const int roadType = ImageStatus.Road_type;
    const uint8_t cirqueFlag = ImageStatus.CirqueOff;

    /* 决策树扁平化提升分支预测 */
    if ((roadType == RightCirque || roadType == LeftCirque) && cirqueFlag == 'F') {
        towPoint = CIRCLE_TOW_POINT;
    } else if (roadType == Straight) {
        towPoint = SystemData.straighet_towpoint;
    } else if (roadType == Cross_ture) {
        towPoint = CROSS_TOW_POINT;
    } else if (ImageFlag.image_element_rings_flag == 1 ||
               ImageFlag.image_element_rings_flag == 2) {
        towPoint = CIRCLE_TOW_POINT;
    } else {
        towPoint = (int)(ImageStatus.TowPoint - speedGain);
    }

    /* 前瞻点约束优化 */
    const int minSafePoint = ImageStatus.OFFLine + 1;
    towPoint = CLAMP(towPoint, minSafePoint, MAX_TOW_POINT);

    // ================ 3.加权偏差计算优化 ======================
    float detValue = (float)ImageStatus.Det_True;
    const int offLine = ImageStatus.OFFLine;

    // 提升变量到 switch 外部
    float weightedSum = 0.0f;
    float totalWeight = 0.0f;

    const int calcCase = (towPoint - PARTIAL_OFFSET >= offLine) ? 0 :
                        (towPoint > offLine) ? 1 : 2;

    switch (calcCase) {
        case 0: { // 全可见区域
            weightedSum = 0.0f;
            totalWeight = 0.0f;
            const int startY = MAX(towPoint - PARTIAL_OFFSET, 0);
            const int endY = MIN(towPoint, LCDH);

            /* 前向加权展开循环 */
            for (int y = startY; y < endY; y++) {
                const int idx = towPoint - y - 1;
                weightedSum += Weighting[idx] * ImageDeal[y].Center;
                totalWeight += Weighting[idx];
            }

            /* 后向加权边界预计算 */
            const int revStartY = MIN(towPoint + PARTIAL_OFFSET, LCDH-1);
            const int revEndY = towPoint + 1;
            for (int y = revStartY; y >= revEndY; y--) {
                const int idx = y - towPoint - 1;
                weightedSum += Weighting[idx] * ImageDeal[y].Center;
                totalWeight += Weighting[idx];
            }

            /* 中心点优化访问 */
            if (SAFE_RANGE_Y(towPoint)) {
                weightedSum += ImageDeal[towPoint].Center;
                totalWeight += WEIGHT_BASE;
            }
            break;
        }

        case 1: { // 部分可见区域
            weightedSum = 0.0f;
            totalWeight = 0.0f;
            const int startY = MAX(offLine, 0);
            const int endY = MIN(towPoint, LCDH);

            /* 正向可见区间循环展开 */
            for (int y = startY; y < endY; y++) {
                const int idx = towPoint - y - 1;
                weightedSum += Weighting[idx] * ImageDeal[y].Center;
                totalWeight += Weighting[idx];
            }

            /* 反向扩展区间优化 */
            const int reverseEnd = 2 * towPoint - offLine;
            const int revStartY = MIN(reverseEnd, LCDH-1);
            const int revEndY = towPoint + 1;
            for (int y = revStartY; y >= revEndY; y--) {
                const int idx = y - towPoint - 1;
                weightedSum += Weighting[idx] * ImageDeal[y].Center;
                totalWeight += Weighting[idx];
            }

            if (SAFE_RANGE_Y(towPoint)) {
                weightedSum += ImageDeal[towPoint].Center;
                totalWeight += WEIGHT_BASE;
            }
            break;
        }

        case 2: { // 边缘场景
            weightedSum = 0.0f;
            totalWeight = 0.0f;
            const int startY = MIN(offLine + 3, LCDH-1);
            const int endY = MAX(offLine + 1, 0);

            /* 倒序循环减少分支 */
            for (int y = startY; y >= endY; y--) {
                const int idx = y - towPoint - 1;
                if (idx >= 0 && idx < sizeof(Weighting)/sizeof(float)) {
                    weightedSum += Weighting[idx] * ImageDeal[y].Center;
                    totalWeight += Weighting[idx];
                }
            }

            if (SAFE_RANGE_Y(offLine)) {
                weightedSum += ImageDeal[offLine].Center;
                totalWeight += WEIGHT_BASE;
            }
            break;
        }
    }

    /* 统一计算结果处理 */
    detValue = (totalWeight > 1e-6f) ? (weightedSum / totalWeight) : 0.0f;

    // ================ 4.状态更新 ======================
    ImageStatus.Det_True = detValue;
    ImageStatus.TowPoint_True = towPoint;
    offect = (float)ImageStatus.Det_True-39;  //中线偏差

    #undef MIN_SPEED_GAIN
    #undef MAX_SPEED_GAIN
    #undef CIRCLE_TOW_POINT
    #undef CROSS_TOW_POINT
    #undef MAX_TOW_POINT
    #undef WEIGHT_BASE
    #undef PARTIAL_OFFSET
    #undef SAFE_RANGE_Y
    #undef MAX
    #undef MIN
    #undef CLAMP
}

//--------------------------------------------------------------
//  @name           Broken_platform_Judgment()
//  @brief          整个图像判断的子函数，用来判断契台类型.
//  @parameter      void
//  @time
//  @Author         PPP
//  Sample usage:   Broken_platform_Judgment();
//--------------------------------------------------------------
void Broken_platform_Judgment(void){
    static uint8 clean ;

    if (
              ImageStatus.Right_Line > 2
            ||ImageStatus.Left_Line  > 2
            || Straight_Judge(2, 25, 45) > 0
        )
            return;

    if (    (ImageDeal[42].IsLeftFind == 'H'
            || ImageDeal[43].IsLeftFind == 'H'
            || ImageDeal[44].IsLeftFind == 'H'
            || ImageDeal[45].IsLeftFind == 'H'
            || ImageDeal[46].IsLeftFind == 'H'
            || ImageDeal[47].IsLeftFind == 'H'
            || ImageDeal[48].IsLeftFind == 'H')
            &&!ImageFlag.Broken_platform_flag
        ){
        ImageFlag.Broken_platform_flag = 1;
        ImageStatus.Road_type = Broken_platform;
    }
    if(ImageFlag.Broken_platform_flag){

        if(!clean){

            jump_flag = 1;
            clean = 1;
        }
        else if(clean){
            clean  = 0;
            ImageStatus.Road_type = 0;
            ImageFlag.Broken_platform_flag = 0;
        }
    }
}
//--------------------------------------------------------------
//  @name           Element_Judgment_Left_Rings()
//  @brief          整个图像判断的子函数，用来判断左圆环类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Left_Rings();
//--------------------------------------------------------------
void Element_Judgment_Left_Rings()
{
    if (
              ImageStatus.Right_Line > 7
            ||ImageStatus.Left_Line < 13
            ||ImageStatus.OFFLine > 10
            || Straight_Judge(2, 25, 45) > 50
            || ImageStatus.WhiteLine>15
            || ImageDeal[52].IsLeftFind == 'W'//如果本行左边界没扫到但扫到的是白色，说明本行没有左边界点
            || ImageDeal[53].IsLeftFind == 'W'//如果本行左边界没扫到但扫到的是白色，说明本行没有左边界点
            || ImageDeal[54].IsLeftFind == 'W'//如果本行左边界没扫到但扫到的是白色，说明本行没有左边界点
            || ImageDeal[55].IsLeftFind == 'W'//如果本行左边界没扫到但扫到的是白色，说明本行没有左边界点
            || ImageDeal[56].IsLeftFind == 'W'//如果本行左边界没扫到但扫到的是白色，说明本行没有左边界点
            || ImageDeal[57].IsLeftFind == 'W'//如果本行左边界没扫到但扫到的是白色，说明本行没有左边界点
            || ImageDeal[58].IsLeftFind == 'W'//如果本行左边界没扫到但扫到的是白色，说明本行没有左边界点
        )
            return;
    int ring_ysite = 25;
    Left_RingsFlag_Point1_Ysite = 0;
    Left_RingsFlag_Point2_Ysite = 0;
    for (int Ysite = 58; Ysite > ring_ysite; Ysite--)
    {
        if (ImageDeal[Ysite].LeftBoundary_First - ImageDeal[Ysite - 1].LeftBoundary_First > 4)
        {
            Left_RingsFlag_Point1_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = 58; Ysite > ring_ysite; Ysite--)
    {
        if (ImageDeal[Ysite + 1].LeftBoundary - ImageDeal[Ysite].LeftBoundary > 4)
        {
            Left_RingsFlag_Point2_Ysite = Ysite;
            break;
        }
    }

    for (int Ysite = Left_RingsFlag_Point1_Ysite; Ysite > ImageStatus.OFFLine; Ysite--)
    {
        if (   ImageDeal[Ysite + 6].LeftBorder < ImageDeal[Ysite+3].LeftBorder
            && ImageDeal[Ysite + 5].LeftBorder < ImageDeal[Ysite+3].LeftBorder
            && ImageDeal[Ysite + 3].LeftBorder > ImageDeal[Ysite + 2].LeftBorder
            && ImageDeal[Ysite + 3].LeftBorder > ImageDeal[Ysite + 1].LeftBorder
            )
        {
            Ring_Help_Flag = 1;
            break;
        }
    }
    if(Left_RingsFlag_Point2_Ysite > Left_RingsFlag_Point1_Ysite+3 && Ring_Help_Flag == 0 )
    {
        if(ImageStatus.Left_Line > 13)
            Ring_Help_Flag = 1;
    }
    if (Left_RingsFlag_Point2_Ysite > Left_RingsFlag_Point1_Ysite+3 && Ring_Help_Flag == 1 && ImageFlag.image_element_rings_flag ==0)
    {

        ImageFlag.image_element_rings = 1;
        ImageFlag.image_element_rings_flag = 1;
        ImageFlag.ring_big_small=1;
        ImageStatus.Road_type = LeftCirque;
    }
    Ring_Help_Flag = 0;
}
//--------------------------------------------------------------
//  @name           Element_Judgment_Right_Rings()
//  @brief          整个图像判断的子函数，用来判断右圆环类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Right_Rings();
//--------------------------------------------------------------
void Element_Judgment_Right_Rings()
{
    if (   ImageStatus.Left_Line > 7
        || ImageStatus.Right_Line < 13
        || ImageStatus.OFFLine > 10
       || Straight_Judge(1, 25, 45) > 50
        || ImageStatus.WhiteLine>15
        || ImageDeal[52].IsRightFind == 'W'
        || ImageDeal[53].IsRightFind == 'W'
        || ImageDeal[54].IsRightFind == 'W'
        || ImageDeal[55].IsRightFind == 'W'
        || ImageDeal[56].IsRightFind == 'W'
        || ImageDeal[57].IsRightFind == 'W'
        || ImageDeal[58].IsRightFind == 'W'
        )
        {return;}
    int ring_ysite = 25;
    Right_RingsFlag_Point1_Ysite = 0;
    Right_RingsFlag_Point2_Ysite = 0;
    for (int Ysite = 58; Ysite > ring_ysite; Ysite--)
    {
        if (ImageDeal[Ysite - 1].RightBoundary_First - ImageDeal[Ysite].RightBoundary_First > 4)
        {
            Right_RingsFlag_Point1_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = 58; Ysite > ring_ysite; Ysite--)
    {
        if (ImageDeal[Ysite].RightBoundary - ImageDeal[Ysite + 1].RightBoundary > 4)
        {
            Right_RingsFlag_Point2_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = Right_RingsFlag_Point1_Ysite; Ysite > 10; Ysite--)
    {
        if (   ImageDeal[Ysite + 6].RightBorder > ImageDeal[Ysite + 3].RightBorder
            && ImageDeal[Ysite + 5].RightBorder > ImageDeal[Ysite + 3].RightBorder
            && ImageDeal[Ysite + 3].RightBorder < ImageDeal[Ysite + 2].RightBorder
            && ImageDeal[Ysite + 3].RightBorder < ImageDeal[Ysite + 1].RightBorder
           )
        {
            Ring_Help_Flag = 1;
            break;
        }
    }
    if(Right_RingsFlag_Point2_Ysite > Right_RingsFlag_Point1_Ysite+3 && Ring_Help_Flag == 0)
    {
        if(ImageStatus.Right_Line>7)
            Ring_Help_Flag = 1;
    }
    if (Right_RingsFlag_Point2_Ysite > Right_RingsFlag_Point1_Ysite+3 && Ring_Help_Flag == 1 && ImageFlag.image_element_rings_flag == 0)
    {

        ImageFlag.image_element_rings = 2;
        ImageFlag.image_element_rings_flag = 1;
        ImageFlag.ring_big_small=1;
        ImageStatus.Road_type = RightCirque;
    }
    Ring_Help_Flag = 0;
}

/****元素检测*****/  //圆环 十字 车库 需要通过编码器积分自己跳出元素标志
void Element_recognze(void) {
  if (  ImageStatus.Road_type == Normol
      &&ImageStatus.Road_type != Cross
      &&ImageStatus.Road_type != LeftCirque
      &&ImageStatus.Road_type != RightCirque
      &&ImageStatus.Road_type != Cross_ture
      &&ImageStatus.Road_type != Broken_platform
      &&ImageStatus.Road_type != Deep
      )
    ImageStatus.Road_type = 0; //赛道类型清0
  //横断
  //Broken_platform_Judgment();
  //圆环检测
  if (ImageStatus.Road_type != Cross_ture)
  {
      Element_Judgment_Left_Rings();           //左圆环检测
      Element_Judgment_Right_Rings();          //右圆环检测
  }
}




/* 图像处理相关常量 */
#define IMAGE_PROCESS_START_ROW         59      // 起始处理行
#define IMAGE_WIDTH_MAX                 79      // 图像最大列索引
#define DEFAULT_OFFLINE                 1       // 默认截止行初始值
#define BOTTOM_LINE_ROW                 (LCDH - 2) // 底边行号
#define INVALID_PIXEL                   0       // 无效像素标记
#define VALID_PIXEL                     1       // 有效像素标记
/**
 * @brief 初始化图像处理数据结构
 * @param startRow 起始行号
 * @param endRow   结束行号
 */

static void InitializeImageData(uint8 startRow, uint8 endRow)
{
    for (uint8 y = startRow; y >= endRow && y < LCDH; y--)
    {
        ImageDeal[y].IsLeftFind = 'F';
        ImageDeal[y].IsRightFind = 'F';
        ImageDeal[y].LeftBorder = INVALID_PIXEL;
        ImageDeal[y].RightBorder = IMAGE_WIDTH_MAX;
        ImageDeal[y].LeftTemp = INVALID_PIXEL;
        ImageDeal[y].RightTemp = IMAGE_WIDTH_MAX;
        ImageDeal[y].close_LeftBorder = INVALID_PIXEL;
        ImageDeal[y].close_RightBorder = IMAGE_WIDTH_MAX;
    }
}


void ImageProcess(void)
{
    if (!mt9v03x_finish_flag) return;  // 等待图像就绪
    compressimage();
    ImageStatus.OFFLine = 2;  //这个值根据真实距离得到，必须进行限制
    ImageStatus.WhiteLine = 0;
    InitializeImageData(IMAGE_PROCESS_START_ROW, ImageStatus.OFFLine);
    Get01change_dajin();
    Pixel_Filter();
    DrawLinesFirst();     //绘制底边    30us
    DrawLinesProcess();   //得到基本边线  8us
    Search_Border_OTSU(Pixle, LCDH, LCDW, LCDH - 2);//58行位底行
    DrawExtensionLine();
    //Element_recognze();
    RouteFilter();//中线滤波平滑 2us
    GetDet();

}
PID_STEER_see pid_image;

// 位置式PID计算函数
double Loc_camera_PID(double expect_Angle, double Actual_Angle, PID_STEER_see *pid)
{
    pid->steer_error = expect_Angle - Actual_Angle;

    // PID计算: P + D
    pid->PID_result = pid->kp * pid->steer_error +
                     pid->kd * (pid->steer_error - pid->steer_error_previous);

    // 保存上一次误差
    pid->steer_error_previous = pid->steer_error;

//    // 限制输出范围 -20 到 20
//    pid->PID_result = CLAMP(pid->PID_result, -20, 20);
    if(pid->PID_result <= -20){
        pid->PID_result = -20;
    }
    if(pid->PID_result >= 20){
        pid->PID_result  = 20;
    }

    return pid->PID_result;
}

// PID控制参数初始化
void Servo_camera_PID_Init(double kp, double kd)
{
    pid_image.kp = kp;
    pid_image.kd = kd;
    pid_image.steer_error_previous = 0;
    pid_image.PID_result = 0;
}

int image_target;
void image_PID(void)
{
    image_target = SERVO_MOTOR_MID + Loc_camera_PID(0, offect , &pid_image);
    Steer_Set(image_target);
}
