《从零开始编写NES模拟器教程》第4课、PPU背景绘制

## 目录

1. 1、前言
2. 2、PPU的结构、运行原理、指令。
    1. 2-1、PPU的结构。
    2. 2-2、图像存储器（pattern memory）。
    3. 2-3、图块（tile）。
    4. 2-3、调色板（palette）。
3. 3、搭建代码框架。
    1.  3-1、创建变量（颜色、图块、名称表）
    2. 3-2、用公式提取图块的像素信息。
    3. 3-3、用公式提取颜色信息。
    4. 3-4、分配显存区域。
    5. 3-5、读取调色板、锁死镜像、读取图案表。
    6. 3-6、增加切换调色板功能、美化界面。
4. 4、首次测试图案表的渲染情况。
    1. 4-1、演示时出现死循环问题。
    2. 4-2、CPU与PPU交互所用的8个寄存器。
    3. 4-3、屏幕渲染基础知识。
    4. 4-4、垂直消隐的介绍。
5. 5、对图案表代码进行修复。
    1. 5-1、配置控制寄存器、掩码寄存器、地址锁变量。
    2. 5-2、注意时钟周期的延迟。
    3. 5-3、配置状态寄存器。
    4. 5-4、纠正前方（4-2）的一个错误。
    5. 5-5、用破解代码暂时解决死循环问题。
    6. 5-6、配置“不可屏蔽中断”功能。
    7. 5-7、图案表的问题得到修复。
6. 6、通过名称表来渲染背景。
    1. 6-1、简单回顾当前进度。
    2. 6-2、核心内容-背景是如何存储与渲染的。
    3. 6-3、水平镜像、垂直镜像。
    4. 6-4、把名称表的渲染过程转换成公式。
    5. 6-5、属性内存的分配过程（子调色板）。
    6. 6-6、配置镜像功能代码。
    7. 6-7、测试名称表的渲染情况。
    8. 6-8、修复扫描枪横向递增与纵向递增的问题。
7. 7、渲染图像。
    1. 7-1、一帧图像的渲染过程《帧时序结构图》。
    2. 7-2、介绍loopy寄存器（vram、tram）。
    3. 7-3、用vram寄存器存储递增信息。
    4. 7-4、用tram寄存器存储名称表信息。
    5. 7-5、在时钟函数配置下一图块的加载时点。
    6. 7-6、添加X轴递增、Y轴递增、X轴重置、Y轴重置代码。
    7. 7-7、移位寄存器如何控制像素移动的。
    8. 7-8、配置移位寄存器的取数，更新像素、调色板的功能。
    9. 7-9、组合出绘制背景的代码。
8. 8、再次测试屏幕渲染情况。
9. 9、结束语。
10. 10、可供参考资料：
11. 11、专用名词：

![](https://picx.zhimg.com/70/v2-7ab8e63ecea5f163215ea2323a02489c_1440w.image?source=172ae18b&biz_tag=Post)

（视频总时长：67分08秒。）

注：本教程超过3万字，且有超过100张截图，费手机流量，建议用电脑端观看。本教程是根据油管大神Javidx9的视频所翻译成的文字版教程，只是用来学习编程技术，非本人原创，侵删。

## 1、前言

（从00分00秒到01分00秒）。

大家好，欢迎收看本系列视频的第4集。如果你还没有看过本系列中的其它视频，那我建议你看了其它视频再来看这个。

在开始之前，我要先做一个小小的免责声明（disclaimer）：我从来没说过这个模拟器会100%遵从原版游戏机的功能（faithful to the original hardware），本模拟器的时钟周期可能不够精确（accurate），也无法完全兼容（isn't compatible with）市面上所有的游戏，但这不是最重要的。

我制作本系列视频的目的，是想讲述一种模拟一个系统的方法（an approach to emulating a system）。碰巧的是，我选择了一个对我意义重大的系统。

本视频是讲解图像处理器（Picture Processing Unit，PPU）的两部视频中的第1部，它是NES游戏机的主要硬件上的一个设备，它负责生成图像并显示在屏幕上给我们看。特别在本视频中，我们将看到NES游戏机在生成背景图像的时候，是如何存储和渲染它的图形的（stores and renders its graphics）。

然后，请允许我占用大家一点时间，在这里我感谢所有在GitHub和油管频道上支持我的朋友。




## 2、PPU的结构、运行原理、指令。

（从01分00秒到8分45秒）

### 2-1、PPU的结构。

（从01分00秒到02分08秒）。

从前面的视频，我们了解到图像处理器（PPU）可以通过它自己的总线（bus）可以连接3个存储器，分别是：

①图像存储器（Pattern Memory，也叫Character ROM，缩写CHR，也叫拼图库、pattern table图案表），容量8KB，地址范围是0x0000~0x1FFF。我准备假设它是只读存储器（ROM）。虽然它也可以是随机存储器（RAM），但是在本项目中，我就把它当做ROM来处理。

②名称表内存（Name Table Memory），容量2KB，地址范围0x2000~0x3EFF。为什么地址范围却接近8KB呢，因为它通过2KB的地址范围进行3次镜像，得到4块相同的内存区域，所以每块都是2KB，加起来就是8KB（which is mirrored through this address range）。并且你也会听到我称它为V-RAM。有时候，这个特殊内存的配置是举足轻重的，我们会详细的进行研究。

③调色板（palette），容量大约是256字节，地址范围0x3F00~0x3FFF，它负责确定屏幕上应该显示哪种颜色。

![](https://pic3.zhimg.com/v2-7129441f5efeda6e7dcea1f2d5f63b06_r.jpg)

*图片（PPU总线上的3个存储器）*

广义上讲，图像存储器（pattern memory）包含了精灵（sprites），就是游戏中各种可行动物体的外观，它们以位图的形式存放（stored as bitmap images）。名称表内存描述了背景的布局（the layout of the background），调色板则存放了颜色。

本视频主要关注游戏背景图像的绘制，因此尽管PPU的内存可以操控精灵，但不要想当然它只能为精灵服务。它只是包含了PPU的所有部件在屏幕上绘制图像所需的图形信息（It just merely contains the graphical information that all parts of the PPU require to draw things on the screen.）。

### 2-2、图像存储器（pattern memory）。

（也叫CHR、Character Rom、拼图库、pattern table图案表）

（从02分08秒到03分42秒）

8KB的图像存储器，总体上可以从中间平分成两块4KB的区域。每个区域又可以划分为16*16的网格（grid of sprites）。每个网格可以称位一个图块（tile），每个图块都是8*8像素大小。这意味着，如果我们把一个4KB的区域看成一张图片，那么这张图片就是128像素宽*128像素高。

![](https://picx.zhimg.com/v2-2f98a4704343439c3690372f4fb28793_r.jpg)

*图片（图像存储器划分成2个区域，即2个拼图）*

![](https://picx.zhimg.com/v2-da115804c731cd9ac2b793a2243dbb09_r.jpg)

*图片（魂斗罗的2个拼图库）*

PPU有能力可以决定是选择左边的区域，还是选择右边的区域，以访问图像的来源（to access the source for its drawing）。

假设图像存储器只存在于卡带中，我们需要通过映射器（mapper）才能调用它。因此映射器能够有选择性地切换两块区域，以访问不同的图块（sprite）。事实上，这就是NES游戏中物体（sprite）能够栩栩如生的原因。

8*8像素的图块，看起来像素并不多，所以我们在游戏或背景层看到的物体通常由多个图块组成。这种内存分割很常见，左边的区域负责处理精灵，即屏幕上左右移动上下跳动的角色；右边的区域负责处理背景层的物体，这些物体组合成游戏中的风景。

永远不要低估一名艺术家在图块数量如此有限的情况下的创造力，在著名的游戏《超级马里奥兄弟》中，云朵和草丛其实是一样的，它们通过改变颜色来重复利用图块。

![](https://pic3.zhimg.com/v2-d1ab8b4a96b0aebeaffb9fb80975670e_r.jpg)

*图片（左边拼图库存放精灵图块，右边拼图库存放背景图块）*




### 2-3、图块（tile）。

（从03分42秒到05分32秒）

图块是一个8*8像素的位图（bitmap），但它不像你在Windows画图软件中看到的那种。NES游戏机给每个像素点只配了2个比特位（bit），所以给了设计师每个像素点4种颜色的选择（即00、01、10、11）。然后它以下面这种非常便捷的方式来存储图块。

既然我们只有4种颜色，我们可以假设像素的值是0、1、2、3，然后一个图块的像素信息会存储在两个位图平面中（stored in two bit planes）。

![](https://pic2.zhimg.com/v2-be2adda433eb195fd1ebc246465a52ad_r.jpg)

*图片（一个图块以两个位图平面进行存储）*

所以我把上图右上角的位图平面称位“副位图平面”（least significant bit plane，LSB），右下角的称为“主位图平面”（most significant bit plane，MSB）。

某个像素点的值，等于两个位图平面在该像素点的值相加的和（The value of a specific pixel is the sum of the two bits from the respective locations in the bit planes.）。所以上图可以看到，图块第三行第二列的像素点的值0，在两个位图平面中该位置都是0。而第三行第三列的1，在副位图是1，主位图是0。然后2，在副位图是0，主位图是1。最后的3，在副位图和主位图都是1。像素值0通常被认为是透明，原因等会大家就会明白。

用位图平面来存放像素值让设计变得方便许多，这意味着我们在提取图像数据的时候，可以节省大量的移位和位操作运算（It means we don't have to do lots of shifting and bit wise manipulation to extract the data.）。

你可能注意到了，8*8像素的位图平面的宽度就是8比特位，相当于1个字节。在内存中，一个图块以是主位图平面+副位图平面的方式进行存储的，这意味着一个图块有16个字节大小（8*16）。我可能应该用十六进制来绘制它们（I probably should have drawn those in Hexadecimal）。

所以我们回到上面的图像存储器，一旦我们确定了需要使用的图块在内存中的偏移地址（the tile offset is in that memory of the tile that we're interested in），我们就能够从内存中把该地址后面的16字节的数据（即图块的数值）读取出来。




### 2-3、调色板（palette）。

（从05分32秒到8分45秒）

图块像素点的2个比特不足以指定它的颜色，我们还要把它跟调色板进行结合。

调色板内存以下图的方式构建：

![](https://picx.zhimg.com/v2-1e0fafe19f358084eb56b80a3d2b53f1_r.jpg)

*图片（调色板内存的构建方式，总共9个地址）*

地址$3F00是一个单独的背景颜色接口，它能够存储一个8比特的值，该值索引了是NES调色板中的一个颜色。屏幕右上角展示的是由程序员Bisqwit所制作的NES调色板。

![](https://pic4.zhimg.com/v2-2f6c8797f7b16a679fded7c95f9b48a3_r.jpg)

*图片（程序员Bisqwit所制作的NES调色板）*

例如，如果我想让背景色为天蓝色（be cyan），我们可以在上图中查找合适的索引值。在这里，我用的就是0x2C，然后把它存放在接口$3F00处，占用1个字节（1个字节=8比特，即1byte=8bit）。

![](https://picx.zhimg.com/v2-1f2b9ac7e469e7ced82ff0ed51f17153_r.jpg)

*图片（给背景接口$3F00设置天蓝色）*

调色板内存的第2个映射地址（$3F01）能够存储4个字节的值，虽然它有4个可能的接口，但是第4个接口是闲置的（unused），让我们看看为什么这样设置。

![](https://pic4.zhimg.com/v2-723c1a02a2eb7aaf8843974ac991c2ff_r.jpg)

*图片（第2个映射地址$3F01）*

首先我们给调色板内存的地址编上序号0~7，分别对应0：$3F01，1：$3F05，2：$3F09，3：$3F0D，4：$3F11，5：$3F15，6：$3F19，7：$3F1D。

如果我们用“选定的调色板ID”与“2个比特的像素值”组合成计算公式，就能够选出用于显示所需颜色的内存地址。

例如，我们假设“调色板ID”是1（palette=1），“像素值”是3（pixel=3）。我们知道每个调色板都包含了4个字节的空间，所以我们要用“调色板ID”乘以4，然后加上“像素值”（即，1*4+3=7）。而且我们知道调色板内存是从$3F00开始的，所以我们从该地址开始往后索引。

![](https://pic3.zhimg.com/v2-955e648b53f929bad978e2610c9fb76e_r.jpg)

*图片（从背景接口开始，依次数0、1、2一直到7）*

然后我们就找到了1号调色板的第3个接口（entry 3 for palette 1），它是由调色板ID和像素值共同决定的。

你可能很好奇，如果“像素值”是0的话，那么（1*4）+0=4，我们数一下，0、1、2、3、4，4刚好就是闲置的接口。所以像素值0意味着该图块的像素是透明的，即直接显示背景颜色。

所以前面我说这个特定的调色板接口是闲置的（unused），其实有点误导大家了，实际上，它是背景色地址的镜像，后面的每个调色板地址的第4个接口都是一样（都是背景色地址的镜像）。

![](https://pica.zhimg.com/v2-33d4bd3f9b0414563bb514211724f5b2_r.jpg)

*图片（每个调色板地址都有4个接口，第4个接口都是透明）*

这种巧妙的程序架构意味着，无论你为特定的绘图选择了什么调色板，你都可以从背景中获得有效的额外颜色（This clever architecture means that no matter what palette you have selected for your particular drawing, you get an effective bonus colour from the background.）。这会让你正在绘制的项目看起来透明。尽管这个功能明显是用于精灵的（前景），但它也可以用于背景。

而且我认为整个游戏机程序就是一个很好的例子，说明设计师们为了计算出给定像素的最终输出颜色，而设计的最优内存策略和最简计算方案，并且还考虑了它是否透明（And I think this whole assembly is a wonderful example of the designers really thinking about the most optimal memory strategies and require the most minimum computation in order to compute the final output color for a given pixel, depending on whether or not it can be seen.）。

调色板内存的最后一块知识点就是，前面的4个内存地址（0：$3F01，1：$3F05，2：$3F09，3：$3F0D）是作为背景调色板的，而后面的4个内存地址（4：$3F11，5：$3F15，6：$3F19，7：$3F1D）是作为前景调色板或精灵的。你会听到我把前景或精灵这两个词等价使用。最后我认为将不同游戏的图像存储器（pattern memory，也叫图案表）显示出来会很有趣。

![](https://pic2.zhimg.com/v2-aa909a3a99c405259779a63a58ee01c1_r.jpg)

*图片（调色板内存的背景地址BG和前景地址FG）*




## 3、搭建代码框架。

（从08分45秒到16分01秒）

我会延续上一集的代码继续编写，但是因为本视频有太多内容，所以我不打算逐字进行讲解。正如《第2课.CPU功能》那样，我也会提供一个附带的源文件，它会详细解释PPU在这个阶段的每个方面。因此，你会发现在观看视频的同时翻阅源文件，会非常有用。

在上一集视频，我们给PPU搭建了一个简陋的框架，它除了显示雪花点之外啥功能也没有。但它能通过0号映射器（mapper 0）完美地连接到CPU总线和游戏卡带。

###  3-1、创建变量（颜色、图块、名称表）

（从09分22秒到09分59秒）

首先我们创建了一个数组palScreen，并用它存储了NES能够显示的所有颜色。然后我们在一个叫sprPatternTable[]的数组中创建了2个拼图库，它将用来显示图像存储器（pattern memory）。

![](https://pic2.zhimg.com/v2-507674894a37f22dd47fcca4f874c83d_r.jpg)

*图片（在PPU代码文件中添加的各种颜色）*

![](https://picx.zhimg.com/v2-5c9e94969bbf8782890db5ee56277b61_r.jpg)

*图片（在PPU头文件中添加的数组和变量）*

```text
//在olc2C02.h文件中给sprPatternTable数组添加的代码：
行42 olc::Sprite sprPatternTable[2] = { olc::Sprite(128, 128), olc::Sprite(128, 128) };
```

所以，现在让我们来实现这个功能，因为我认为编写的过程可以真正理解调色板与位图是如何协同工作的（So let's implement this function，because I think by doing this will gain a real understanding of how palette and the bitmaps work together.）。

我们知道，对于一个给定的图案表中（pattern table图案表，就是图像存储器，里面有2个拼图，一个是精灵拼图，另一个是背景拼图），有16*16个图块（tiles），所以我会创建2个嵌套的for循环来遍历这些图块（iterate through）。我还想把二维坐标转换成一维坐标来索引图像存储器（pattern memory，也叫拼图库）里面的图块。




### 3-2、用公式提取图块的像素信息。

（从09分59秒到12分40秒）

现在我们用图块的“Y轴坐标nTileY”乘以宽度（width）再加上“X轴坐标nTileX”，这样就把二维转换成了一维。这个公式我在许多视频中说过许多次了，但这次有点不一样。这里我们有nTileY，但是宽度是256像素，这是因为一个图块（a single tile）包含了16个字节的信息，而在图像存储器的拼图库中（sprite memory）每一行有16个图块。所以这个变量nOffset存放的是内存中的字节偏移量（uint16_t nOffset=nTileY*256+nTileX*16;）。

```text
//在olc2C02.cpp文件中给GetPatternTable函数添加的代码：
行95  olc::Sprite & olc2C02::GetPatternTable(uint8_t i)
行96  {
行97      for(uint16_t nTileY = 0 ; nTileY < 16; nTileY++)
行98      {
行99        for(uint16_t nTileX = 0 ; nTileX < 16 ; nTileX++)
行100       {
行101         uint16_t nOffset = nTileY * 256 + nTileX * 16;
```

对于每个图块，有8行，每行有8个像素，所以为了完成它，我会在这里增加一个遍历col的循环。

```text
//在olc2C02.cpp文件中给GetPatternTable函数添加的代码：
行103  for(uint16_t row = 0; row < 8; row++)
行104  {
……
行108    for(uint16_t col = 0; col < 8; col++)
行109    {
```

为了能够从图像存储器（pattern memory，也叫拼图库）中读取数据，我们要用到ppuRead()函数，它会把一个地址送入PPU总线，然后读取到需要数据。然后映射器将会帮我们翻译数据，并轻松地计算出目标地址。

我们知道在图像存储器中有2个拼图可供选择，我们根据要读取的拼图库把参数i传递给ppuRead()函数，然后函数计算出该选哪个拼图（ppuRead(i)）。

我们还知道，完整的图案表是4KB（ppuRead(i * 0x1000)），在这个4KB里面我们需要把刚才计算出来的nOffset作为偏移量加上去（ppuRead(i * 0x1000 + nOffset)）。

我们知道，在每个图块中（tile），每一行的像素有1个字节大小，所以也要偏移1行（ppuRead(i * 0x1000 + nOffset + row)）。

而且因为我们知道，程序会先读取“副位图”（least significant bit，LSB），然后才读取“主位图”（most significant bit，MSB）。为了让公式长度相等，我会在“副位图”的取数公式tile_lsb的row后面加个0。相对应的，“主位图”的取数公式tile_msb也是一样的，不同的只是row后面加个8。

```text
//在olc2C02.cpp文件中给GetPatternTable函数添加的代码：
行105  uint8_t tile_lsb = ppuRead(i * 0x1000 + nOffset + row +0x0000);
行106  uint8_t tile_msb = ppuRead(i * 0x1000 + nOffset + row +0x0008);
```

上述2个公式会计算出2个字节的结果给我，每个字节包含8个像素的数值。然后我们需要把这些字节组合起来算出最终的位图颜色，它介于0~3之间。我会把这2个字节相加，但我只对它们的最低感兴趣（即二进制的最右边1位），因为相加的结果就是0~3（即二进制的00、01、10、11）。

```text
//在olc2C02.cpp文件中给GetPatternTable函数添加的代码：
行110  uint8_t pixel = (tile_lsb & 0x01) + (tile_msb & 0x01);
```

这意味着下一次col循环，我们还需要进行同样的运算，因此，我们需要把所有二进制位向右移1位，这样，下次循环右边的倒数第2位就成为了最低位。

```text
//在olc2C02.cpp文件中给GetPatternTable函数添加的代码：
行111  tile_lsb >>= 1; tile_msb >>= 1;
```

现在我们有了一个像素的值，那么我们就能开始把该值在图像存储器中找到对应的拼图，并绘制出来（We can start to draw that value into the sprite that represents this section of the pattern memory.）。

指定图块的X坐标，就是图块在像素空间的X偏移量。但我让它加上7、再减去col的值，是因为我们的“图块的二进制数的最低位”指的是“图块最右边的像素”，但我们首先会从图块的左上角开始绘制，所以我就需要在X轴上反转过来。

```text
//在olc2C02.cpp文件中给GetPatternTable函数添加的代码：
行113  sprPatternTable[i]->SetPixel
行114  (
行115    nTileX * 8 + (7 - col);
……
行118   );
```

而Y轴就简单多了。

```text
//在olc2C02.cpp文件中给GetPatternTable函数添加的代码：
行116  nTileY * 8 + row;
```




### 3-3、用公式提取颜色信息。

（从12分40秒到13分34秒）

最后，我们需要选择颜色，但截止目前还没有用过调色板，手头上只有那2个比特的像素值。所以我将创建并调用一个叫做GetColourFromPaletteRam()的函数，它会接收“调色板ID”和“像素值”并计算出最终显示在屏幕的颜色。

```text
//在olc2C02.cpp文件中给GetPatternTable函数添加的代码：
行117  GetColourFromPaletteRam( palette , pixel )
```

这也意味着，我需要给GetPatternTable函数增加一个palette参数。

```text
//原来在olc2C02.cpp文件中的代码：
行95  olc::Sprite & olc2C02::GetPatternTable(uint8_t  i )
//修改后的代码：
行95  olc::Sprite & olc2C02::GetPatternTable(uint8_t  i，uint8_t  palette )
```

接下来，让我们创建这个GetColourFromPaletteRam()函数的代码。还记得吧，我们需要得到调色板ID，并让它乘以4（相当于二进制数左移2位），然后给它加上像素值ID，然后把整个值偏移到调色板内存中。并且还需要调用ppuRead()函数计算出内存中的地址。由ppuRead()函数返回的值，是NES系统的颜色调色板中一个索引号（index），我们会把它存入palScreen数组中（上文中提到，该数组存放的是NES系统支持的所有颜色）。因此我们可以直接给GetColourFromPaletteRam函数返回颜色的结果。

```text
//在olc2C02.cpp文件中给GetColourFromPaletteRam函数添加的代码：
行130  olc::Pixel& olc2C02::GetColourFromPaletteRam(uint8_t palette, uint8_t pixel)
行131  {
行132    return palScreen[ppuRead(0x3F00 + (palette << 2) +pixel)];
行133  }
```




### 3-4、分配显存区域。

（从13分34秒到14分02秒）

当前，ppuRead()与ppuWrite()函数都还没有填充代码，它们不会起任何作用，所以我们现在把3个主要的内存添加进来（图像存储器、名称表内存、调色板）。

我们知道，我们可以根据游戏卡带的映射器对所请求地址进行重新定位（We know that we can defer to the map around the cartridge to handle relocations of the requested addresses.）。图像存储器（pattern memory）坐落在内存地址$0000~$1FFF区域，而名称表（name table）坐落在$2000~$3EFF区域，最后调色板内存（palette memory）位于$3F00~$3FFF区域。

```text
//在olc2C02.cpp文件中给ppuRead函数添加的代码：
行189  uint8_t olc2C02::ppuRead(uint16_t addr,bool rdonly)
行190  {
行191    uint8_t data = 0x00;
行192    addr &= 0x3FFF;
行194    if (cart->ppuRead(addr,data))
行195    {
行197    }
行198    else if (addr >= 0x0000 && addr <= 0x1FFF)
行199    {
行200    }
行201    else if (addr >= 0x2000 && addr <= 0x3EFF)
行202    {
行203    }
行204    else if (addr >= 0x3F00 && addr <= 0x3FFF)
行205    {        
行206    }
行208    return 0;
行209  }
```

![](https://picx.zhimg.com/v2-baa710b79fcaf98948c104adb069e423_r.jpg)

*图片（给ppuRead()函数填充的代码）*

ppuRead()函数有的内存区域，ppuWrite()函数也同样有。

![](https://pica.zhimg.com/v2-87e5f9bde50cdee6dd88a7f46f3bdfac_r.jpg)

*图片（给ppuWrite()函数填充的代码，从219~227行）*




### 3-5、读取调色板、锁死镜像、读取图案表。

（从14分02秒到15分06秒）

在PPU类中（olc2C02.h头文件），我们已经把它们当作数组创建好了，名称表数组（tblName[2][1024]）、调色板数组（tblPalette[32]）、图像存储器数组（tblPattern[2][4096]）。

![](https://pic2.zhimg.com/v2-e360f8850b1a547acef56a08d692c0b3_r.jpg)

*图片（在olc2C02.h头文件中添加的3个数值变量）*

```text
//在olc2C02.h头文件中给添加的代码：
行15  private:
行16    uint8_t  tblName[2][1024];
行17    uint8_t  tblPalette[32];
行18    uint8_t  tblPattern[2][4096];
```

对于调色板数组（tblPalette[32]），我们可以使用屏蔽功能留下最右边的5个二进制位，来提取合适的索引值（For the table palette array,we can select the appropriate index by masking the bottom five bits.）。（即，让参数addr跟0x001F进行【与运算】，十六进制数0x001F相当于二进制数的0000-0000-0001-1111）。

```text
//在olc2C02.cpp文件中给ppuRead函数添加的代码：
行206  addr &= 0x001F;
```

并且，我会在代码中锁死镜像功能取数的地址（hard-coding the mirroring，硬编码）。

```text
//在olc2C02.cpp文件中给ppuRead函数添加的代码：
行207  if (addr == 0x0010) addr =0x0000;
行208  if (addr == 0x0014) addr =0x0004;
行209  if (addr == 0x0018) addr =0x0008;
行210  if (addr == 0x001C) addr =0x000C;
```

最后只需从该内存地址读取数据即可。

```text
//在olc2C02.cpp文件中给ppuRead函数添加的代码：
行211  data = tblPalette[addr];
```

而且自然对ppuWrite()函数也要配置相似的操作。

![](https://pic3.zhimg.com/v2-bb7c34f10375af95f048ea8c5e9e1ae8_r.jpg)

*图片（给ppuWrite()函数填充的代码）*

既然我们想把图像内存进行可视化（visualize pattern memory），那么让它能够被读取将是有用的。在这个内存地址的操作十分简洁。这个是图像存储器数组（tblPattern），数组的第1维通过检查PPU地址的最高二进制位来确定是选择左边的拼图还是右边的拼图（Here is the pattern memory array, and the first dimension chooses whether it's the left or the right hand side of that array of data by examining the most significant bit of the PPU address.）（即，[(addr & 0x1000) >> 12]）。数组的第2维通过屏蔽PPU地址中剩余的二进制位来计算该内存中的偏移量（The offset into that memory is calculated by masking the remaining bits of the PPU address.）（即，[addr & 0x0FFF]）。

```text
//在olc2C02.cpp文件中给ppuRead函数添加的代码：
行200  data = tblPattern[(addr & 0x1000) >> 12][addr & 0x0FFF];
```

图像存储器内存通常是一个ROM（只读存储器），但是为了以防万一，我还是把它添加到ppuWrite()函数中，因为在某些游戏卡带中，这块内存实际会是RAM（随机存储器）。

![](https://pic3.zhimg.com/v2-16de7742c0076dac1dd0131e49547420_r.jpg)

*图片（在ppuWrite()函数中添加的代码）*




### 3-6、增加切换调色板功能、美化界面。

（从15分06秒到16分01秒）

让我们回到主程序，现在，大家所见到的是《像素游戏引擎》的衍生产品，我们能够用它来显示图像，并与模拟器交互。

![](https://pic2.zhimg.com/v2-4049084f5a25b2a23e6e2108661e3905_r.jpg)

*图片（像素游戏引擎中的代码）*

对于这个类（像素游戏引擎），我会在其中添加一个变量nSelectedPalette（代码：uint8_t nSelectedPalette=0x00;），因为我想让用户通过点击键盘上的P键，即可选择不同的调色板来绘制图像内存的内容。

![](https://pic3.zhimg.com/v2-0ea28d7914080a9f9955b345b93d5d52_r.jpg)

*图片（在像素游戏引擎代码中添加的变量nSelectedPalette）*

![](https://pic3.zhimg.com/v2-761588f57366266b4a63f273adc40634_r.jpg)

*图片（设置了点击P键进行切换的功能）*

这个功能只是把变量的值递增，从0直到7，之后又会返回从0开始递增。

现在，绘制图像内存很简单，我们只需绘制精灵，并调用PPU类中的GetPatternTable()函数就能即时渲染出图像内存（We just draw the sprite and we call the PPU function “GetPatternTable()” to render the pattern table instantaneously for us.）。而且我想同时看到图像内存中的2个拼图，所以我同时给它们传递了参数0和1，并且还传递了当前所用调色板的索引号（nSelectedPalette）。

![](https://pica.zhimg.com/v2-27d25ead52b91f029aef6d15dd76e500_r.jpg)

*图片（在像素游戏引擎中添加的绘制精灵的函数）*

事实上，代码就在下图中，我不打算详细描述它，我只想告诉大家，这段代码会在显示调色板的同时告诉我们被选定的是哪个。

![](https://pic2.zhimg.com/v2-b8b3c3fd55cf98e1f40edc6940ddfa05_r.jpg)

*图片（添加的绘制图像内存的代码）*

```text
//在olcNes_PPU_Backgrounds.cpp文件中添加的代码：
行239  const int nSwatchSize = 6;
行240  for (int p = 0; p < 8; p++) // For each palette
行241	 for(int s = 0; s < 4; s++) // For each index
行242	 FillRect(516 + p * (nSwatchSize * 5) + s * nSwatchSize, 340, 
行243	 nSwatchSize, nSwatchSize, nes.ppu.GetColourFromPaletteRam(p, s));
行245	DrawRect(516 + nSelectedPalette * (nSwatchSize * 5) - 1, 339, (nSwatchSize * 4), nSwatchSize, olc::WHITE);
```

不用担心，上述代码跟模拟器没有关系，它纯粹就是美化用户界面的。




## 4、首次测试图案表的渲染情况。

（从16分01秒到23分34秒）

### 4-1、演示时出现死循环问题。

（从16分01秒到16分58秒）

现在让我们来看一看。

![](https://pic2.zhimg.com/v2-067b0f25debee61963ec63d236e77323_r.jpg)

*图片（点击运行Local Windows Debugger后出现的界面）*

好，现在我们能够看到右下角有2个灰色的矩形，它们代表了图像内存中的2个拼图。如果我点击键盘的P键，那么右下角的光标就会在8个不同的调色板之间切换。

![](https://picx.zhimg.com/v2-000d6b3a120d3c6c942fb4b6feb16f79_r.jpg)

*图片（程序被卡在$C009~$C00C之间）*

好的，地址$2002，实际上地址$2000~$2007都是重要的寄存器。它们负责控制PPU并把状态推送给CPU。




### 4-2、CPU与PPU交互所用的8个寄存器。

（从16分58秒到19分21秒）

在我们继续运行程序之前，我们需要看看这些寄存器。

CPU通过8个寄存器与PPU进行通信，实际是9个，但第9个我们会在下一集视频再介绍。

在CPU地址总线上，这8个寄存器所在的地址是$2000~$2007,尽管它们在更大内存地址中有镜像。

①但是$2000就是控制寄存器（CTRL），它负责配置PPU以不同的形式进行渲染（This is responsible for configuring the PPU to render in different ways.）。

②在地址$2001，我们有一个掩码寄存器（MASK），它决定了程序正在绘制的是背景还是精灵，以及屏幕边缘发生的事情（This decides whether backgrounds or sprites are being drawn and what's happening at the edges of the screen.）。

③对于地址$2002很重要，它是状态寄存器（STATUS），在本视频中，它很重要是因为它会告诉我们什么时候才能安全地渲染图像。

④后面两个地址（$2003、$2004），在本视频中我也先省略不讲。

⑤在地址$2005，我们有滚动寄存器（SCROLL），正是通过这个寄存器，我们能够描绘出比屏幕大得多的游戏世界。正如马里奥向右跑向旗子时，水平面的图像其实是向左滚动的。

⑥最后，在地址$2006和$2007，我们这两个寄存器让CPU能够直接读取或者写入PPU内存地址的数据（Finally, we have two registers in $2006 and $2007 that allow the CPU to directly read and write to the PPU’s memory address and data.）。PPU完整的寻址范围是14比特（14-BITS），但CPU每次进行写入数值都只能传输8比特，所以它必须连续两次写入才能设置完整的地址，第一次传输低8位，第二次传输高8位（油管大神表示：此处有错误，后面进行更正。正确的做法是先设置高字节，再设置低字节）。

![](https://pic3.zhimg.com/v2-37e82f66125215a9e15b6079e6ee3cae_r.jpg)

*图片（CPU与PPU进行传输的8个寄存器地址）*

当然，实际要传输的数据都是通过数值寄存器进行写入或读取的。对于PPU头文件（olc2C02.h），我打算添加一些结构体（struct），它们包含了位字段（bit-field）可以表示重要的寄存器。

下面这个是状态寄存器，它有3个很重要的二进制位，第一个是垂直消隐（vertical_blank），第二个是精灵零点（sprite_zero_hit），第三个是精灵溢出（sprite_overflow）。然后我把它们组合成一个8位的寄存器变量，这样我们就能以数值形式访问它。请参阅本系列的第一集，了解这种结构是如何运作的。

![](https://pic2.zhimg.com/v2-62f9be4e657a1feee44f63beb5fa7db5_r.jpg)

*图片（在olc2C02.h头文件中添加的状态寄存器）*

掩码寄存器（mask register）就是一系列的开关，它决定了PPU的每个部件是打开还是关闭，因此从下图的代码中我们可以看到很重要的二进制位“渲染精灵（render_sprites）”和“渲染背景（render_background）”。

![](https://picx.zhimg.com/v2-cc24afc7f6c5e31b9d313676ca6f9bb7_r.jpg)

*图片（在olc2C02.h头文件中添加的掩码寄存器）*

最后，我们还有控制寄存器（control register）。

![](https://pic2.zhimg.com/v2-31662a811b36f18c17e4f0520af42811_r.jpg)

*图片（在olc2C02.h头文件中添加的控制寄存器）*




### 4-3、屏幕渲染基础知识。

（从19分21秒到20分50秒）

我完全理解，要学的新东西太多了，但我会一边拆解程序一边解释新名词。

刚才我运行的测试程序被卡在循环中，是因为它想从状态寄存器读取数据，特别是垂直消隐的状态（uint8_t vertical_blank:1）。

现在让我们花点时间来了解一下渲染一帧图像时所发生事件的顺序。

![](https://pic3.zhimg.com/v2-c8adb87873cfd4384d80db70541377b4_r.jpg)

*图片（屏幕的示意图）*

在上一集视频，我创建了2个奇怪的变量“扫描线scanline”和“周期cycles”，它们相当于我们屏幕上的Y轴坐标值和X轴坐标值，在我们的屏幕上，它们之间肯定有关联，但不是1比1的。

扫描线（Scanlines）代表了屏幕上水平方向的行，在以前电视机的原理就是“用电子枪射出电子，冲击屏幕上的磷光材料（phosphorescent material），通过磷光材料发光来显示图像的”。

NES游戏机屏幕的分辨率是“宽256像素”乘以“高240像素”，不过事实上扫描线能够超出屏幕的尺寸，因为扫描线将会跨越它的计数周期（However, the scanline can exceed these dimensions as the scanline is going across its counting cycles.）。并且我们可以粗略认为“一个计数周期”相当于“扫描线上的一个像素”。

![](https://picx.zhimg.com/v2-6fc99ba09dfcc78ddfd57468041026ff_r.jpg)

*图片（屏幕上的扫描线可以超出屏幕范围，只是你看不到）*

既然扫描线可以超出屏幕范围，那么周期计数（cycle count）也可以。事实上，每条扫描线上就有341个计数周期，这是个大概数（approximate）。如果你真的去仔细阅读NESDEV开发文档，你就会发现有些数字有少量的四舍五入（some of these numbers are rounded slightly）。




### 4-4、垂直消隐的介绍。

（从20分50秒到23分34秒）

一旦电子枪到达了扫描线的最右端，它就会关闭，然后回车到下一行的最左端开头再开始扫描。并且它会顺着屏幕往下，不断的横向扫描、回车到下一行、横向扫描、再回车，因此在可见区域有240条横向扫描线。但当它到达底部时也不会停止，事实上在屏幕底部之外还有一些扫描线，总共是有261条横向扫描线（屏幕内240条+屏幕外21条）。绘制这些看不见的扫描线（21条）所经历的时间，就叫做垂直消隐期（vertical blanking period）。

![](https://pic1.zhimg.com/v2-c4d84502b2d9164ad47b94a375b4a1c6_r.jpg)

*图片（横向扫描线261条，每条线有341个计数周期）*

游戏必须要知道垂直消隐期的开始时点，如果电子枪仍在屏幕内绘制扫描线的同时，CPU却开始与PPU进行交互，它可能会无意中导致屏幕出现各种图形伪影。虽然在某些高华丽场景，它事实上是高效利用系统资源。但是在我们的简单案例中，它会产生干扰，它看起来就不好看。

因此，在屏幕绘制的时候，CPU可以用这段时间来可以做一些数据处理，但重要的是它不能去改变PPU的属性。不过一旦垂直消隐期开始了，CPU当然就可以去修改PPU的属性了，反正我们在屏幕上看不到它做了啥，它随心所欲的捣乱都行。所以，通常在这段时间里，CPU会设置好PPU为下一帧图像做准备。

根据我们设计本模拟器的初衷，我们会假设，一旦到达了最后的扫描线，我们不会返回到0号扫描线。相反，我们放置一条“负1号扫描线”在屏幕上方（稍后我们开始渲染背景图块的时候，我们会看到更多关于它的信息）。

![](https://pic1.zhimg.com/v2-9be50995f8dbda09fd0b5145cf503e4c_r.jpg)

*图片（在屏幕上方放置一条负1号扫描线）*

所以，在状态寄存器变量中的“垂直消隐比特位”（即【uint8_t vertical_blank:1;】这段代码所设置的二进制位），会告诉我们，当前是在屏幕绘制状态，还是虚无状态。我们可以啥都不干，当然，我们也可以选择在这个时间点向CPU发生一个中断请求，这里可以利用6502的“不可中断请求”指令的特性（Non-Maskable-Interrupt feature of the 6502）。

![](https://pica.zhimg.com/v2-92868b6fa4aa02d690f20b45e34dc4f2_r.jpg)

*图片（在左下角的垂直消隐即将开始时，确定是否中断）*

是否发送中断请求，都是由PPU寄存器上的这个比特位所控制。“垂直消隐比特位”与“不可中断请求指令”的组合，是用来同步PPU和CPU的，这样屏幕上的图像才能让人看起来正常。CPU完成工作的时候，屏幕必须同步渲染图像是很重要的，否则图像会出现延迟。而且，CPU可能需要等待一整帧结束才能更新屏幕。在NES游戏中，当屏幕上的东西变得忙碌时，你就会看到上述情况发生；在现代游戏中也会发生类似情况，这就是原因。




## 5、对图案表代码进行修复。

（从23分34秒到31分48秒）

### 5-1、配置控制寄存器、掩码寄存器、地址锁变量。

（从23分34秒到24分51秒）

在本视频的前面，我们给这些寄存器留了空，现在是时候给它们填充代码了。

①让我们从cpuWrite()函数的控制位寄存器（Control）开始，我们把正在写入的数值分配给控制位寄存器。

![](https://pica.zhimg.com/v2-5b0648aefa766fe6c3a7282daea8a5c8_r.jpg)

*图片（给cpuWrite()函数的控制状态标志位添加代码）*

![](https://pica.zhimg.com/v2-cda847e72d94846ab127e27954506c16_r.jpg)

②因为这样做很方便，所以我会对掩码寄存器（Mask）做同样的操作。

```text
//在olc2C02.cpp文件中给cpuWrite()函数添加的代码：
行173 case 0x0001:  //Mask
行174  mask.reg = data;
行175  break;
```

③你无法对状态寄存器（Status）写入数值，而且我们在本视频暂不考虑这2个寄存器（OAM Address和OAM Data）。

④对于滚动寄存器（Scroll），我们等会再来看。

⑤为了操作地址寄存器（PPU Address）和数据寄存器（PPU Data），我需要一些变量。我需要知道我正在给高字节还是低字节写入，所以我创建一个叫做地址锁（address_latch）的变量，表明高字节还是低字节。当我们从PPU读取数据时，事实上会占用一个时钟周期，所以我们需要缓冲这个字节的周期。我还需要一个16比特的变量来保存编译好的内存地址。

![](https://picx.zhimg.com/v2-c667f15f027743e0d1576576832c2e91_r.jpg)

*图片（在olc2C02.h头文件中添加的3个变量）*

所以如果我们正在对地址寄存器（PPU Address）进行写入，并且地址锁等于0，那么，我会把新的PPU地址的低字节存入变量（ppu_address），并把地址锁设置为1，这样就为下一次存入高字节做了准备。

![](https://pic4.zhimg.com/v2-b9f9a8b6da6bd9c59b70106b24d0a443_r.jpg)

*图片（在olc2C02.cpp文件中给cpuWrite()函数的地址寄存器添加的代码）*

一旦我获得了完整的地址，那我就能使用ppuWrite()函数给数值寄存器（PPU Data）写入数据。

```text
//在olc2C02.cpp文件中给cpuWrite()函数添加的代码：
行196  0x0007:  //PPU Data
行197  ppuWrite
行198  break;
```




### 5-2、注意时钟周期的延迟。

（从24分51秒到26分32秒）

在cpuRead()函数中，你是不能从地址寄存器（address register）读取数据的，因为这不是本函数的职责。不过我们可以从数据寄存器（data register）读取数据，但它会被一次读取操作延迟了，所以我会把“缓存变量ppu_data_buffer”当前存放的数值传送给“输出变量data”，并把当前地址的值存入“缓存变量ppu_data_buffer”。

![](https://pic4.zhimg.com/v2-7d26ddc9c51f6a0e46c89c8e2be6bb77_r.jpg)

*图片（在olc2C02.cpp文件中给cpuRead()函数添加的代码）*

在这里，我们会遇到许多模拟程序怪事（many quirks of emulation）中的第一个，就是，这个读取延迟会对所有PPU地址范围产生影响，除了调色板内存（This delayed read is true for almost all of the PPU address range except for our palette reside.）。出现这种情况的硬件原因有很多，在当前所有东西都是与PPU的指令时钟同步的，因此，在内存可以输出值之前，它需要一个地址来进行初始化，这些都需要时钟周期（And so before memory can output the value，it needs to be primed with an address，these all take clock cycles.）。这就是出现延迟的原因。

然而，有些类型的小内存不需要这种延迟，它们使用组合逻辑，并且可以在相同的时钟周期内输出数据。这需要一点时间，但作为设计者，通常你可以确保，该电路产生正确结果的传播时间在时钟周期内。在NES模拟器中，我觉得调色板内存就是按这种方式存储的。因此，我们需要为调色板地址添加一个特殊处理。因此，在本案例中，我不想等待另一个时钟周期才提取缓存变量中的数值。

```text
//在olc2C02.cpp文件中添加的代码：
行163  if(ppu_address > 0x3F00)  data = ppu_data_buffer;
```

我知道你们中的一些人会提前考虑，并意识到这些操作在真正的模拟器中都不可能奏效。你们是对的，但是我们会在实现背景滚动功能的时候再回来解决。




### 5-3、配置状态寄存器。

（从26分32秒到27分40秒）

在读取函数中还有一个我们感兴趣的寄存器，就是状态寄存器（status register）。从状态寄存器中读取也会对PPU执行某些操作，这个概念有点让人摸不着头脑。仅仅是读取操作就会改变设备的状态。当我们读取状态寄存器（status register）的时候，我们只关注最高的3个比特位。其它比特位只需填充无效数值，或者更可能是PPU内部数据缓冲区上一次存储的数据。

![](https://pic4.zhimg.com/v2-1af64a370dce9a34c996b7e744a307ad_r.jpg)

*图片（olc2C02.cpp文件中给cpuRead函数添加的代码）*

我认为所有正版NES游戏都不依赖于上述操作，事实上它只在NES开发者中使用，所以我也把功能配置在这里，其实你们也可以完全不使用这个功能。 

有趣的是，读取状态寄存器（status register）也会清除垂直消隐标志（vertical blank flag），所以无论你是否处于垂直消隐期都无所谓，只要你读取了状态以确定是否处于垂直消隐，就让标志位清零。除了设置垂直消隐标志，读取状态寄存器也会让地址锁（address_latch）清零。

```text
//在olc2C02.cpp文件中添加的代码：
行149  case 0x0002:  //Status
行150  data = (status.reg & 0xE0) | (ppu_data_buffer & 0x1F);
行151  status.vertical_blank = 0;
行152  address_latch = 0;
行153  break;
```




### 5-4、纠正前方（4-2）的一个错误。

（从27分40秒到27分59秒）

稍等一下，这让我想起来好像做错了什么。在我们给地址端口（address port）写入地址的时候，我先设置低字节，再设置高字节，这样是错误的，我犯了个错。正确的做法是先设置高字节，再设置低字节。所以我要把代码中的高低位交互顺序，非常抱歉。

```text
//原来在olc2C02.cpp中给cpuWrite()函数添加的代码：
行193  case 0x0006:  //PPU  Address
行194  if (address_latch == 0)
行195  {
行196  ppu_address = (ppu_address & 0xFF00) | data;
行197  address_latch =1;
行198  }
行199  else
行200  {
行201  ppu_address = (ppu_address & 0xFF00) | data;
行202  address_latch = 0;
//修改后的代码：
行201  ppu_address = (ppu_address & 0x00FF) | (data << 8);
```




### 5-5、用破解代码暂时解决死循环问题。

（从27分59秒到29分44秒）

我们还记得，程序被卡在读取状态寄存器的指令中，所以我打算先插入一条破解代码（So I'm going to hack in something.），只是为了让程序往前运行，我打算让程序每次读取状态寄存器时，垂直消隐标志位都变为1。这样程序就能通过原先被卡住的地方了。

![](https://picx.zhimg.com/v2-82fd9fcfb8c9176db94c84c9806a09b5_r.jpg)

*图片（在olc2C02.cpp文件中给cpuRead函数插入的一条破解代码，做临时用）*

然后我们运行程序看一看。

![](https://pic3.zhimg.com/v2-00aa4ef58c2f32a459cea85dcf3d8362_r.jpg)

*图片（再次运行程序，终于有东西显示在右下角了）*

我会点击运行程序，然后，好极了，把它暂停一下。

现在我们能够看到在图像内存（pattern tables）的拼图库中有一些图块信息，但它看起来有点不对劲。这是测试NES模拟器的ROM文件，我也知道这看起来不像，不应该是这样的颜色，调色板也不太对。而且我们能够看到，程序并没有努力让调色板变得有用。

我们遗漏了一个相当重要的部分，就是当CPU给PPU写入数据的时候发生了什么。总是要程序员把“2字节的地址”和“1字节的数据”写入PPU，是很繁琐的。正常情况是，程序员把数据写入连续的地址，然后PPU为处理这些数据提供一种设备，这种设备能够在实施写入指令（cpuWrite）和读取指令（cpuRead）时自动递增PPU内存地址。

![](https://pica.zhimg.com/v2-402f32443eb670be8983cd289aeb8036_r.jpg)

*图片（在olc2C02.cpp文件中给cpuWrite函数添加的地址递增代码）*

![](https://pic2.zhimg.com/v2-a96cd64ebf0386876a70e2789567368d_r.jpg)

*图片（在olc2C02.cpp文件中给cpuRead函数添加的地址递增代码）*

然后我们再运行程序试试。

现在好多了，我们可以看到调色板有各种颜色。并且如果我按键盘的P键，就可以让光标切换调色板，与此同时图像内存也会根据选定的调色板而变化。

![](https://pic1.zhimg.com/v2-0dbb9e3d5982586c45cec949fa05ac66_r.jpg)

*图片（再次运行程序，右下角的调色板有不同的颜色了）*

![](https://pic1.zhimg.com/v2-e281534b96d52bc00c2f99c7882235c8_r.jpg)

*图片（按P键后切换调色板的效果）*

![](https://pic4.zhimg.com/v2-150d45498308f11c7262f6adc0b9a383_r.jpg)

*图片（按P键后切换调色板的效果）*

为了给大家演示，所以我极大的简化了模拟器在此处的代码（指olc2C02.cpp文件中的cpuWrite和cpuRead函数等代码）。在本视频的后半部分，我们将会看到，根据扫描线的位置来选择图块填充屏幕的工作将会有多么的复杂。




### 5-6、配置“不可屏蔽中断”功能。

（从29分44秒到31分02秒）

我们现在即将进入背景渲染的复杂部分。但是，在我们进入之前，让我们配置一下在每一帧画面的末端定时发送“不可屏蔽中断（non-maskable-interrupt）”的功能。

我们知道进入垂直消隐的准确时点：就是当我们开始绘制第241号扫描线（scanline==241），并且时钟周期（cycle==1）开始计时的时候。在这个时点，我会让状态寄存器的垂直消隐标志位等于1（status.vertical_blank=1;）。如果控制寄存器中的“不可屏蔽中断生效标志位（enble_nmi）”的值为1（不是0，就是1），那么我会让“不可屏蔽中断变量（nmi）”的值设置为真（true）,这个变量是布尔型变量，我已经在头文件中创建了它。

![](https://pic2.zhimg.com/v2-156bd68d29f697746d169456819e07bb_r.jpg)

*图片（在olc2C02.cpp文件中给clock时钟函数添加的代码）*

![](https://pic4.zhimg.com/v2-cb828fff87983f179e6fe41447756cf7_r.jpg)

*图片（在olc2C02.h头文件中添加的变量）*

我们已经知道，中断请求是由PPU发送给CPU的，而我将在要总线中实现这个功能。这很简单，如果变量nmi的值为真（ppu.nmi==true），那么我就调用CPU类中的NMI()函数，并且重置PPU上的nmi标志位。

![](https://pic3.zhimg.com/v2-718db52e550c929dfe373263c5d1419e_r.jpg)

*图片（在bus.cpp文件中添加的判断语句）*

我们也知道离开垂直消隐期的时点，因为这个时候我们实际是在屏幕的左上角（即负1号扫描线的起点）。所以我会把状态寄存器的垂直消隐标志位（status.vertical_blank=0;）设为0，因此现在状态寄存器能够准确反映出垂直消隐期的状态了，我们可以自主控制是否发送“不可屏蔽中断”信号了，而我之前手工添加的临时代码也可以删掉了。

![](https://picx.zhimg.com/v2-e9bac982f18dd6537be6ad3b97d8b0cd_r.jpg)

*图片（在olc2C02.cpp文件中添加的判断语句）*

![](https://pic2.zhimg.com/v2-9853f327dbe97bd052c197501213d1ef_r.jpg)

*图片（原来手工添加的临时代码也可以删除了）*




### 5-7、图案表的问题得到修复。

（从31分02秒到31分48秒）

我认为，如果现在显示某个NES游戏的图像内存将会很有趣。下面我们看看《超级马里奥兄弟》的图像内存。

![](https://pica.zhimg.com/v2-edb229cc660c759139cad2b64812910a_r.jpg)

*图片（把《超级马里奥兄弟》的文件名输入到代码中，然后运行程序）*

你几乎可以辨认出某个意大利水管工的精灵图像。它肯定不是吉里奥。

![](https://pic3.zhimg.com/v2-35a80a7f1e1e10c63473aff609baadec_r.jpg)

*图片（程序加载显示的《超级马里奥兄弟》的图像内存）*

在上图中，左边的拼图库，我们可以看到角色的精灵图块（the sprites of the characters）。右边的拼图库，我们可以看到组成背景的拼图块（the tiles that make up the background scenery）。如果我切换调色板，这些图块的颜色也会随之改变。

让我们来试试另一款游戏《大金刚》（donkey kong.nes）。这里我们可以再次看到背景图块和角色精灵图块，并且它们自己会改变颜色，但我并没有切换调色板，是这款游戏它自己切换了调色板。这是个激动人心的事情。

![](https://pic3.zhimg.com/v2-1d2c945635abdba313af8f357651894a_r.jpg)

*图片（加载了另一款游戏之后显示的图像）*




## 6、通过名称表来渲染背景。

（从31分48秒到49分21秒）

### 6-1、简单回顾当前进度。

（从31分48秒到32分32秒）

所以，让我们快速回顾一下我们的进展，因为后续会变得很复杂。

首先，我们给名称表内存、调色板内存、图像内存创建了固定的内存空间。

```text
//已在olc2C02.h头文件中写好的代码：
行15  private:
行16  uint8_t  tblName[2][1024];
行17  uint8_t  tblPalette[32];
行18  uint8_t  tblPattern[2][4096];
```

然后，我们创建了3个寄存器。状态寄存器告诉我们渲染过程所达到的状态（The status register tells us where we're up to in our rendering process.）。掩码寄存器我们还没有仔细讲解。还有控制寄存器，目前我们只讲了如何让“不可屏蔽中断是否生效”标志位（enable_nmi pin）发送正确的中断信息。

```text
//已在olc2C02.h头文件中写好的代码：
行100  uint8_t enable_nmi : 1;
```

我还临时创建了一个叫做ppu_address的寄存器，这样我们就能在程序运行的时候观察图像内存的显示情况。

```text
//已在olc2C02.h头文件中写好的代码：
行109  uint16_t ppu_address = 0x0000;
```

事实上，我们在上面的代码中，对这个地址的工作方式的描述还是太简单了，下一步我们就来仔细看看。

### 6-2、核心内容-背景是如何存储与渲染的。

（从32分32秒到35分13秒）

既然我们已经研究了图形信息是如何存储在NES游戏中的，那么我们准备学习本视频的核心内容，看看背景是怎么存储和渲染的。

在游戏中，背景通常构成关卡场景，与前景中被称为精灵的物体相比，背景几乎是静态的。游戏背景存放在名称表内存的某个名称表中。下图中有一个名称表，它有1KB大小，然后它横向有32个格子、纵向也有32个格子，每个格子有1字节大小，并且这一个字节代表了我们前面的图像内存拼图中的一个图块ID。

![](https://pic2.zhimg.com/v2-858be6e4aa4b39cf8fdb530f70f593a1_r.jpg)

*图片（名称表的示意图）*

还记得吗，图像内存是一个16*16的网格，每个网格是一个图块，每个图块是8*8像素，所以总共有256个图块（16*16=256）。所以很方便的是，名称表上的每一个格子，都有256个图块可供选择。

由于每个图块是8*8像素，所以我们似乎看到了任天堂的图像分辨率是256*256像素（解决方案），因为名称表的横向是32格子*8像素=256像素，纵向也是32格子*8像素=256像素。但不是，实际上它的分辨率是256*240像素，这意味着名称表中有2行格子没有用上（2格子*8像素=16像素）。实际上，名称表底部的2行格子用在了其它地方，我们稍后会讲到。

![](https://picx.zhimg.com/v2-508dee91c0af695615a9bba1854bcedf_r.jpg)

*图片（任天堂实际使用的名称表空间是256*240）*

在制作游戏关卡时，设计师们会细心挑选所需的图块，然后放置在心仪的位置。有时背景甚至包含你可能会认为是前景的元素。在最简单的情况下，一个下图这样的名称表，就是游戏中的全部背景，在初期的任天堂游戏中就是这样，例如大金刚。

![](https://picx.zhimg.com/v2-7ce6edcd386d6092f9f8a53d4cfa5fb3_r.jpg)

*图片（用名称表演示大金刚游戏的背景）*

![](https://picx.zhimg.com/v2-71d9866f610382809a722cf75f719e3b_r.jpg)

*图片（FC大金刚游戏截图）*

稍微复杂一点游戏，例如《马里奥》就需要屏幕从左往右滚动，并且我们可以通过代码把图块偏移到PPU的滚动寄存器中（we can write how many tiles are offset from the top left into the scroll register of the PPU），即名称表右边背景的图块坐标改到屏幕的左边（见下图，这样右边的背景就移动到了屏幕左边）。

![](https://pic4.zhimg.com/v2-d047b134921b4290d876abac28f8fa2d_r.jpg)

*图片（红框表示屏幕，背景在向右滚动）*

![](https://pic4.zhimg.com/v2-a0a22dfa9d12f30a4a8afc7ceed6346b_r.jpg)

*图片（红框表示屏幕，背景向右移动，但是名称表资源不足无法显示）*

但是问题来了，我们显然已经把名称表用完了（这样屏幕右边就没东西可以显示了）。

为了实现屏幕滚动的效果，实际上NES游戏机存储着2张名称表，并且在内存中是相邻的。当屏幕的可视区域滚动，并且准备跨越2个名称表的中间时，我们会同时用两个不同的名称表对屏幕进行渲染。

而此时CPU的任务就是把接下来要显示的游戏关卡背景的数据更新到名称表的不可见区域。

![](https://pic2.zhimg.com/v2-e9fb2e05de3d69fe6acc95404ea8f3dd_r.jpg)

*图片（屏幕滚动的示意图，跨过2个名称表）*

当可视窗口滚动到第二个名称表的末尾时，它会迅速把背景绕回到第一个名称表的左边（When the viewable window scrolls past the end of the second name table, it’s effectively wrapped background into the first one.）。这样你就会感觉到游戏背景的左右移动都是连续的。

![](https://pic4.zhimg.com/v2-d2fceb5474d7c5987803d64da516c7cb_r.jpg)

*图片（当取景框到达第二个名称表末尾时）*

![](https://pic2.zhimg.com/v2-67c625b1cd46b51ac5adc99002d6a97b_r.jpg)

*图片（把取景框绕回到第一个名称表）*




### 6-3、水平镜像、垂直镜像。

（从35分13秒到37分12秒）

NES游戏机内存只能容纳2个名称表，所以显存（vedio RAM）只有2KB。但是通过利用地址镜像功能（address mirroring），我们理论上可以有4个名称表，但游戏机仍然只有2KB用来存储数据，所以另外2个镜像名称表只是复制品（duplicates）。这种复制功能比较难理解，我们称之为镜像（mirroring）。

![](https://pica.zhimg.com/v2-3e45728b32b81fc06b0dc8adb7e95cd4_r.jpg)

*图片（2个名称表+2个复制品）*

下图的这种配置，叫做水平镜像（horizontal mirroring），因为我们的实际内存中存放的是左边的2个名称表（带*号），右边的2个就是镜像（带M符号）。所以，如果我给左上角的名称表写入数据（画个+号），那就等于在右上角的镜像名称表中也写入了数据（也画了+号）。

![](https://picx.zhimg.com/v2-8a0d307a5b4906e0ca0faef803e8e4c3_r.jpg)

*图片（水平镜像示意图）*

《马里奥》游戏屏幕是水平方向滚动的，而有些游戏只能垂直方向滚动，所以那些在上下方向的名称表中滚动屏幕的游戏用的是水平镜像（horizontal mirroring）。

像《超级马里奥兄弟》这样的游戏用的是垂直镜像（vertical mirroring），它用2KB的内存来储存这些名称表，并且2个镜像名称表是在下面（underneath）。在这种情况下，对右上角的名称表写入数据，那么系统会同步给右下角的镜像名称表也写入数据。

![](https://pic1.zhimg.com/v2-8fb4a83c1c062628c9f4cacc9abbbc62_r.jpg)

*图片（垂直镜像的示意图）*

名称表镜像功能的配置可以有多种来源，在某些游戏卡带中例如《超级马里奥兄弟》，它就是刻录在卡带的电路板上（it's hard coded into the cartridge circuit.），这意味着《超级马里奥兄弟》的所有游戏场景都只能水平移动。

有些映射电路（mapping circuits）也能够控制镜像功能实施的方式，并且可以根据需要，动态的进行切换。诚然，这种功能要在更高级的游戏中才有。例如在《塞尔达》中，你能看到屏幕有时左右、有时上下两种方向移动。在更高级的游戏例如《超级马里奥兄弟3》中，你还能看到屏幕同时可以左右上下移动。这里要理解的核心理论就是，当屏幕往某个方向滚动的时候，CPU必须要把即将显示的背景更新到对应的名称表中。

### 6-4、把名称表的渲染过程转换成公式。

（从37分12秒到40分30秒）

我们可以用滚动寄存器指定图块的偏移量（offset）。正如视频的前半部分我们用过许多次的公式，计算公式与“Y*width+X”没什么区别，但如果width是2的n次方（2ⁿ），我们就要利用现有的功能，用二进制计算来执行。

![](https://pica.zhimg.com/v2-7782b2cbe639730d5bca6196ccfe4316_r.jpg)

*图片（计算偏移量的公式，把二维坐标转换成一维的偏移量）*

在这里我们知道，在名称表中有32*32个图块（tiles），因此每个图块的偏移量（offset）需要用一个5比特的二进制数表示（2的5次方=32，用二进制数表示就是00000、00001……11111）。我会称这些偏移量为“coarse y”和“coarse x”，这2个变量都是5比特位，通过简单拼接的方式得到一个10比特位的值，我们实际上完成了这个公式。因为在二进制算法下，如果我们把这个10比特位的值从中一分为二，那么我们可以认为左边的值（左边的11111）就是右边数值（右边的11111）的出现次数（见下图，Because in binary, if we split the word at some point, the number on this side is a count of how many times of this side occurs.）。所以，如果它是两个边长的幂，在这种情况下，它是32。左边自动乘以了32（X32），然后加上右边的任意值（+….）。

![](https://pic3.zhimg.com/v2-7c98eee5ce039306ab61298197a90cf8_r.jpg)

*图片（把二维坐标转换成一维偏移量的公式的推导过程示意图）*

既然我们有了4个可用来寻址的名称表，那么我还要再增加2个比特位（an additional two bits）。不好意思，我又重复用了刚才的网格示意图（刚才是1个名称表，现在把它当做4个名称表）。

因此，使用之前相同的逻辑，我会把10比特位的数值拓展为12比特位，增加“名称表Y（NTy）”和“名称表X（NTx）”两个比特位。

![](https://pica.zhimg.com/v2-5c45776354fb6af82b71ecc303769868_r.jpg)

*图片（把偏移量的值再增加2比特位）*

4个名称表给了我们4096个可能的地址（每个表是1KB大小，4KB=4096字节），刚好就是我们的12比特位地址所能表示的最大数字（2的12次方=4096）。

如果我们的游戏是以整个图块滚动的，那么游戏画面看起来就会很生硬且不连贯（blocky and jumpy），而实际上NES游戏看起来是很流畅的（quite smoothly），因为每个单元（cell）只有8*8像素这么小。

我们还需要把偏移量（offset）存放到一个独立的单元中（a single cell）,因此我需要介绍另外2个变量“fine y”和“fine x”，请注意这2个变量不是“12比特位地址（twelve bit address）”的一部分。

当扫描线在屏幕上快速移动的时候，我们通过计算时钟周期（cycle）来分析出扫描线当前所在的图块是哪个（As the scanline is zipping across the screen, we count cycles to work out which particular tile the scanline is currently residing within.）。扫描线的一个周期相当于一个像素的绘制时间，因此每经历一个周期，我们就把变量“fine x”加1（finex++）。如果“fine x”的值大于7（>7），那么我们就把变量“coarse x”的值加1（coarsex=5变成coarse=6）。

同样的，每当（一行扫描线完成扫描）屏幕出现新扫描线时，我们就把变量“fine y”的值加1（finey++）。如果“fine y”的值大于7（>7），那么我们就把变量“coarse y”的值加1（不管它现在是多少），这样并不会改变变量的比特位，只是把它的值加1而已。

![](https://pic2.zhimg.com/v2-8b2f9f0894f1320859837734a466c7e7_r.jpg)

*图片（屏幕绘制过程中，偏移量的计算过程）*

如果我们横向滚动屏幕，随着扫描线计数周期的增加，在某个时候，我们将向右边进入另一个名称表。在这个时点，我们就可以转置（invert）我们的“名称表X的比特位（NameTablex bit）”（把NTx=1变为NTx=0）。

同样的是，当我们向下滚动屏幕的话，我们就会转置“名称表Y的比特位”（把NTy=1变为NTy=0）。

而且更进一步，如果我们继续往同样的方向滚动屏幕，并且超出了第2个名称表，那么我们就把这2个比特位（即NTx或NTy）再次转置，这样就会把它们重置会正常状态（回到第1个名称表），并实现背景环绕的功能（implements the wrap around functionality）。

### 6-5、属性内存的分配过程（子调色板）。

（从40分30秒到43分44秒）

我们等一下再看看这种寻址方式的细节，可能有些朋友可能已经在思考了，但是请先等一下，我们先讲完调色板。我们已经知道了如何从图像内存中挑选一个图块，然后放置到名称表的指定地址，但是我们还没有指定调色板。

大家回想一下，名称表底部的2行格子在屏幕上是不可见的，这64字节的内存叫做属性内存（attribute memory），每1个字节都对应着名称表中的一块区域（a region of the name table）。既然有64个字节，那么我们可以合理的把名称表假设成是一个8*8=64个网格（grid）。每个字节的属性内存对应着4*4个图块（名称表是32*32个格子组成，划分为8*8的网格，那么每个网格就是32/8=4，即4*4个格子，每个格子使用一个图块）。

![](https://pic2.zhimg.com/v2-119dbb05c0f510fc8c00840e5e721ca3_r.jpg)

*图片（把名称表分成8*8=64个网格）*

这4*4=16个图块在屏幕上占据了不小的空间，而这4*4的空间只能使用一个“子调色板”（每个子调色板只有4种颜色，子调色板存放的是“总调色板”颜色的索引号，而名称表中的属性内存格子存放的是子调色板的索引号），这种难度相当考验设计师。回顾调色板内存，背景的每个图块只能用4个“子调色板”，所以理论上，每个图块的“子调色板”索引号只需使用2比特的内存空间（00、01、10、11，So in principle, we only need two bits per palette.）。

既然属性内存的一个格子就是1字节（名称表内存的一个格子就是1字节），就有8比特大小，我们就可以把这1个字节拆分成4个不同的“子调色板索引”（每个索引就是2比特）。然后我们就可以合理的假设，我们能够把4*4=16的图块区，细分成四个2*2=4的小图块区（four lots of two by two tiles）。

NES游戏机会这样分区，它把该字节的第7比特位和第6比特位（即**01** 111011的最左边2位）分配给小图块区的右下角，把第5第4比特位（01**11** 1011）分配给小图块区的左下角，把第3第2比特位（0111**10** 11）分配给右上角，把第1第0比特位（011110**11** ）分配给左上角。

![](https://pic1.zhimg.com/v2-75fa5898cf828164d91ac0ffa5c61c56_r.jpg)

*图片（把属性内存的1个字节分配给4个小图块区）*

因此，这4个图块（上图用绿色框圈起来的那4个图块）必须共享相同的“子调色板”。我举一个相同尺寸的物体作为参考，例如《超级马里奥兄弟》里面的问号块（question mark block），就是一个2*2的小图块区。

![](https://pic2.zhimg.com/v2-a2dbb5a0ff670ec99d34cfe13b9834cd_r.jpg)

*图片（问号块就是由4个图块组成）*

因此，我们不仅要找到扫描线的位置，以及屏幕滚动后名称表上要显示的图块ID，还要根据相同的信息，为该小图块区选择合适的子调色板，尽管需要对该信息进行少量压缩，例如丢弃某些比特位和偏移到不同的地址（And so not only do we have to work out where our scanline is, after scrolling in the name table to get the tile I.D., we can use the same information, albeit crushed down slightly by throwing some out and offsetting it to a different location to choose the appropriate attributes byte for that region.）。

我们可以把变量“coarse y”和“coarse x”的5比特位压缩成3比特位，然后组合成复合地址来解决上述问题，5比特位压缩成3比特位就跟除以4是一样的。所以我们名称表上原来的32个图块，现在依据属性内存被重新划分成8个子调色板区域（And we can work this out by taking our composited address of coarse y and coarse x, and reducing their five bit implementations to three.This is the same as dividing by four.So our original 32 tiles on the name table have now been reduced to the 8 regions of palette in the attribute memory.）。

![](https://pic3.zhimg.com/v2-2964c98e516cdd65b6ee60cf9a7980fc_r.jpg)

*图片（把变量压缩，把32格子划分成8个区）*

如果我们假设这里（名称表的左上角原点）是0，那么无论我们得到什么数字，它都包含了对属性内存的偏移量。我们就需要对属性内存的起始位置，即3C0也进行偏移（并找到对应的属性内存）。而且我们还要用到刚才丢弃的2个比特位来协助我们在对应的8比特属性内存中，根据给定的图块找到需要的那2个比特位。（If we assume here is address zero for our name table, then clearly whatever number we have got, that represents our attribute memory offset. We need to offset to the start location of the attribute memory, which is three c zero. And we'll use the two bits that we've just thrown out here to help us choose which two bit section of the eight bit attribute memory word, we're using for given tile.）。

![](https://pic4.zhimg.com/v2-47a2cb8270d73ab7fe824d8030157309_r.jpg)

*图片（根据屏幕图像偏移，找到属性内存的偏移，以确定颜色）*

就我个人而言，我发现NES模拟器的这部分内容可能是最复杂的，而且我在这里对它做的描述太简化了，可能会存在问题。因此，请大家务必查阅源文档，因为该文档的评论区有很多关于此机制如何工作的详细信息。

### 6-6、配置镜像功能代码。

（从43分44秒到45分18秒）

在我们开始渲染背景之前，我认为先把名称表可视化比较有用，但我不打算可视化全部图形细节，因为这里只是为了渲染背景。相反，我要做的是显示图块的ID（display the tile IDs），以确保我们正确的设置了所有内存。

这里是我们的2个1KB的名称表（uint8_t tblName[2][1024];）。我们已经在ppuRead和ppuWrite函数中保留了内存空间，用于连接名称表内存，并附带合适的镜像功能，但我们需要知道当前的镜像模式是哪种（水平镜像、垂直镜像）。但对于我正在用来演示的简单游戏，镜像模式信息是存储在卡带中的（contained in the cartridge），所以可以在代表ROM文件头信息的结构体变量中寻找（the structure that represented the header for the ROM）。

![](https://pic2.zhimg.com/v2-880d932c51fe3c13a8755bce4a9dd551_r.jpg)

*图片（代表ROM文件头信息的结构体变量）*

通过使用与“查找游戏所使用映射器型号”类似的方法，我也可以分析出卡带的镜像模式。因此，现在我打算设定4种基本镜像模式：水平镜像模式、垂直镜像模式、以及另外两种未来需要关注的模式。

![](https://pic4.zhimg.com/v2-8de72c1a5de5c9ce7b03106edf47b117_r.jpg)

*图片（在卡带头文件中设定4种镜像模式）*

所以我会根据不同的镜像模式，设定不同的处理方案。有一些花哨的数学方法可以做到这一点，但正如我所描述的，我试图让代码看起来更简洁。

如果我们是在垂直镜像模式下，那么我们就要查看名称表内存起始位置的地址偏移，然后根据该地址选择合适的物理名称表（If we're in vertical mirroring mode, then we look at the address offset to the start of the NameTable RAM and choose the appropriate physical NameTable, depending on that address.）。

![](https://picx.zhimg.com/v2-9ea7a09d8a1f9a2e8226b5a8fc8b1887_r.jpg)

*图片（在olc2C02.cpp文件中给ppuRead函数添加的垂直镜像的处理代码）*

如果是水平镜像模式，其实代码差不多，只需改变物理名称表的顺序。

![](https://pic3.zhimg.com/v2-de1c13a704255f0b65dbb22035c572a2_r.jpg)

*图片（在olc2C02.cpp文件中给ppuRead函数添加的水平镜像代码）*

并且跟往常一样，既然给ppuRead函数填充了以上代码，那也要同步给ppuWrite函数也填充上。

```text
//在olc2C02.cpp文件中添加的代码（行280~行304）
行267  void olc2C02::ppuWrite(uint16_t addr, uint8_t data)
行268  {
……
行279  else if (addr >= 0x2000 && addr <= 0x3EFF)
行280	{  addr &= 0x0FFF;
行281		if (cart->mirror == Cartridge::MIRROR::VERTICAL)
行282		{
行283			// Vertical
行284			if (addr >= 0x0000 && addr <= 0x03FF)
行285				tblName[0][addr & 0x03FF] = data;
行286			if (addr >= 0x0400 && addr <= 0x07FF)
行287				tblName[1][addr & 0x03FF] = data;
行288			if (addr >= 0x0800 && addr <= 0x0BFF)
行289				tblName[0][addr & 0x03FF] = data;
行290			if (addr >= 0x0C00 && addr <= 0x0FFF)
行291				tblName[1][addr & 0x03FF] = data;
行292		}
行293		else if (cart->mirror == Cartridge::MIRROR::HORIZONTAL)
行294		{
行295			// Horizontal
行296			if (addr >= 0x0000 && addr <= 0x03FF)
行297				tblName[0][addr & 0x03FF] = data;
行298			if (addr >= 0x0400 && addr <= 0x07FF)
行299				tblName[0][addr & 0x03FF] = data;
行300			if (addr >= 0x0800 && addr <= 0x0BFF)
行301				tblName[1][addr & 0x03FF] = data;
行302			if (addr >= 0x0C00 && addr <= 0x0FFF)
行303				tblName[1][addr & 0x03FF] = data;
行304		}
```

在展示名称表ID之前，我先把之前添加的雪花点代码注释掉。

```text
//在olc2C02.cpp文件中的旧代码
行338  //Fake some noise for now
行339  sprScreen.SetPixel(cycle-1,scanline[(rand()%2)?0x3F:0x30]);
//修改后的代码
行338  //Fake some noise for now
行339  //sprScreen.SetPixel(cycle-1,scanline[(rand()%2)?0x3F:0x30]);
```




### 6-7、测试名称表的渲染情况。

（从45分18秒到47分28秒）

我想我们已经完成了。因为我没打算把名称表完全可视化，我只是想在屏幕上绘制出名称表，并在对应的名称表中显示图块的ID（Because I don't want to fully visualize the name table, I'm just going to draw the IDs, in the corresponding name table load location atop of whatever the screen is outputting.）。在这种情况下，我选定了0号名称表，并把它们转换成十六进制（hex），然后我还加载了《NESTest》游戏ROM文件，现在让我们看看。

![](https://pic3.zhimg.com/v2-9eb9f2f06a11493453712053a5cd53bc_r.jpg)

*图片（在代码中指定名称表地址，并转换成十六进制）*

所以，一开始还没有东西显示，名称表显示的都是0，现在我运行这个模拟器，然后我们就能看到屏幕上名称表的内容变化了。图像上的数字看起来有一些结构了，但仍然有点难以解释。让我把模拟器先暂停一下。

![](https://pic3.zhimg.com/v2-258e3caac085aaebfff2823f7d00c3bc_r.jpg)

*图片（运行模拟器出现的名称表）*

所以我们可以看到画面上出现了很多20的字符，并且因为所有内容都直接转换成十六进制了，而且我们的图像存储器（右下角的2个拼图库）是16*16的网格，所以我们可以很容易在右下角找到对应的图块。

因此，对于编号20，在右下角的左边拼图库里面找，从左上角数第0行、第1行、第2行，然后第2行的第0格，我们看到目前图块是空白的。

![](https://picx.zhimg.com/v2-ce926e3cd08fc1745db2b5358ba827c9_r.jpg)

*图片（编号20的图块在拼图库中的位置）*

所以屏幕上的图像大部分是空白，这个结果是正确的，因为《NESTest》ROM文件运行时显示的菜单屏幕大部分都是空白的，中间有一个文本列表。

绘制名称表最快速直接的方式，就是直接拿屏幕上的ID，然后使用“像素游戏引擎”中的DrawPartialSprite()函数，在图像存储器中找到对应的精灵图块，然后绘制在屏幕上正确的位置。

你知道吗，我实在忍不住绘制出真正的图像，所以我刚刚设置了下面这行代码，再看看屏幕显示的内容。

![](https://pica.zhimg.com/v2-44563a7d63b39f2194a1b01a7dce3a3a_r.jpg)

*图片（像素游戏引擎中的DrawPartialSprite()函数）*

这次完全有希望显示名称表，但它并没有按照NES游戏机的设定进行显示。但我们还是看到了ROM文件的测试菜单，它应该也能切换调色板。

![](https://picx.zhimg.com/v2-cdce518bbb91b2e8bdac7e2159754fa7_r.jpg)

*图片（调用了DrawPartialSprite()函数后显示了预期的画面）*

好极了，因为我觉得自己有点贪婪，我实在忍不住想尝试其它游戏（donkey kong.nes）。

![](https://pic3.zhimg.com/v2-00a67223fd433b80f38e6c8a2c2f281c_r.jpg)

*图片（运行了大金刚游戏后的画面）*

好吧，我认为程序没有选到正确的名称表，不过精灵图块看起来有点像。我只好使用蛮力把它调整过来（先点右上角X掉游戏，然后点Local Windows Debugger按钮运行程序）。

![](https://pic3.zhimg.com/v2-b01f08243d6cdfef4a9598500f473676_r.jpg)

*图片（再次运行程序后显示的图像）*

好吧，图像好一点了，但还是有点问题，目前看起来有点糟糕，但看到图像能自动运行，我还是觉得很高兴。让我们看看能否修复其中的一些故障，因为我觉得我知道是什么导致了这些问题。

### 6-8、修复扫描枪横向递增与纵向递增的问题。

（从47分28秒到49分21秒）

基本上，模拟器是通过CPU对PPU写入数据的方式更新名称表的。为了让这个过程更有效率，PPU地址会自动递增，所以你只需持续写入数据流即可。然而，PPU地址加1只会在屏幕的水平方向递增地址。如果你想在垂直方向给名称表写入数据该怎么做？好吧，这正是我们设计NES模拟器的目的。 

控制寄存器（control register）有一个特殊的比特位可以由程序控制，叫做递增标志位（increment_mode），这个标志位决定了是加1还是加32。当标志位等于1时，PPU地址是沿着X轴递增的。但如果标志位是32的话，那么PPU地址会沿着X轴一次性跳过32个图块，这跟沿着Y轴下移一行是相同的功能。

![](https://pic2.zhimg.com/v2-1450a4afe0abad909ce0abf3f490cb8f_r.jpg)

*图片（在olc2C02.cpp中给cpuWrite函数配置的控制寄存器）*

![](https://pic4.zhimg.com/v2-42e68c071d59944b30df058298e5b4d3_r.jpg)

*图片（把olc2C02.cpp文件中ppuWrite函数中的++替换成红框的内容）*

既然现在CPU能够控制PPU地址递增的方向了，那么我们再看看刚才的故障修复了没有。噢，非常好，问题解决了，现在屏幕上的画面看起来很合理了。刚才屏幕显示画面失败的原因，是因为我给每个图块都生成一次图案表，所以我马上破解了这个问题，我知道我讲得有点离题了，我只是比较激动。

不过，现在让我们看看《大金刚》是如何渲染图像的，它比之前渲染的速度快得多。在屏幕上我们可以看到游戏的关卡，噢，这就是大金刚本体，额，程序还在制作动画。

![](https://picx.zhimg.com/v2-ac7d378a099b74b19b57dd71f4829165_r.jpg)

*图片（游戏引擎渲染的大金刚游戏）*

我们看看能否选择一个让它看起来像大金刚的调色板吧，应该是这个吧，所以看到这样完成度的画面让我很高兴。

![](https://pic2.zhimg.com/v2-e8fd95e2064c73209477ff539b78310d_r.jpg)

*图片（切换调色板后的大金刚）*

所以，尽管上面看起来像是我们渲染出了背景，但实际上并没有，这完全是黑客行为（hack），因此我们现在要做的就是正确实现背景渲染功能。

所以不要感到困惑，我会把DrawPartialSprite()函数注释掉，我会暂时保留它，万一后续出现问题的时候还会用它来调试程序。

![](https://pic4.zhimg.com/v2-09bc2c7c053713fc0cfd881fc0a97143_r.jpg)

*图片（把DrawPartialSprite()函数注释掉）*




## 7、渲染图像。

（从49分21秒到65分22秒）

### 7-1、一帧图像的渲染过程《帧时序结构图》。

（从49分21秒到52分01秒）

对屏幕进行渲染，需要计算扫描线（scanlines）和周期（cycles），这个在前面学习过了。你能在NESdev维基百科（[http://wiki.nesdev.com](http://wiki.nesdev.com)）找到的最有用的图表之一，就是这个《NTSC PPU Frame Timing》帧时序结构图（http:[http://wiki.nesdev.com/images/4/4f/Ppu.svg](http://wiki.nesdev.com/images/4/4f/Ppu.svg)，本网址已失效）（新网址：[https://www.nesdev.org/wiki/File:Ppu.svg](https://www.nesdev.org/wiki/File:Ppu.svg)）。

此结构图上展示给我们的，是屏幕上横向的周期和纵向的扫描线（the cycles going across the screen and the scanlines going down），并且还告诉我们需要实施什么操作（what operation needs to be performed）。别忘记了，屏幕上的1个周期代表1个像素，所以8个周期就代表一个图块中的1行（So eight cycles represents one row of one tile.）。PPU只能够存储它要渲染的下一个图块的信息，所以在这8个周期中，程序会加载后面8个像素所需的信息。

![](https://picx.zhimg.com/v2-70bade786cd942ea1c6089c91fa4aafb_r.jpg)

*图片（在维基百科网站中的结构图）*

（在上图第239行）在这种情况下，它会先加载名称表字节（NT byte，NameTable byte），也就是图块ID号；然后加载属性字节（AT byte，attribute byte），它包含了调色板信息；最后加载图块自己（Then it loads the pattern itself.），这里要记住，图块会被分成两部分，一部分代表背景图块的副位图字节（LSB，Low BG tile byte），另一部分代表背景图块的主位图字节（High BG tile byte）。

它知道应该读取那个图块，因为它已经读取了名称表字节（NT byte）；它也知道如何组合成正确的颜色，因为它已经读取了属性字节（AT byte）。一旦绘制完这8个像素，我们就会马上着手准备下一个图块的数据。上图中水平方向数字不断递增的格子说明了这个过程（This movement is illustrated by this increased horizontal cell in the chart.）。然后我们不断重复这个过程，在屏幕水平方向上不断读取和绘制图块。

屏幕上有256个像素点，所以一行像素的绘制工作到这里就停下了。即使一行扫描线最后的8个像素也遵循相同的模式，但是它们的数据不会被程序使用，因为超出屏幕之外绘制扫描线是没有意义的，因为根本看不到（Even though the last eight pixels of a scanline follow the same pattern, the data is unused because there's no point in rendering beyond the scanline.）。并且别忘记了，周期的数量也超过了屏幕上像素的数量。

![](https://pic3.zhimg.com/v2-3d11119bc91699bdfd7d019ae543ea20_r.jpg)

*图片（第256个像素点之后就不会再进行绘制操作）*

我们真正要做的事情，就是为绘制下一行扫描线的第1个图块，而准备好渲染系统。而这个准备工作发生在扫描线的末端（就是下图321~340的位置）。

![](https://pic1.zhimg.com/v2-201a46d16bd17e5fbb2cf6a09232e374_r.jpg)

*图片（一行扫描线的末端，321~340就是为绘制下一行做准备的地方）*

准备工作中会重复读取图块ID，但它们是无效的（ignored）。你可能会想到，在绘制动作开始之前，我们会先回到负1号扫描线，额，跳转到负1号扫描线的目的（在下图中就是0号标签），只是为了给屏幕上的第1个像素做准备。

![](https://pic2.zhimg.com/v2-0c9db984241da9b98bbdc186320ad761_r.jpg)

*图片（0号标签）*

所以，当扫描线在屏幕上横越时，在每个图块的边界，我们会递增内存地址，其实就是访问名称表（NT byte）。当我们到达可见区域的边缘时，我们也会增加内存地址，但我们是在名称表的垂直方向增加（Y轴坐标加1），并且把X轴坐标重置为0。

这个《帧时序结构图》特别好的地方，就是它还包括了一些额外信息、我们前面讲过的信息，例如设置垂直消隐标志（vblank flag）的时间点，以及加载精灵的时间和地点，但在本视频中我们暂时不讲。

### 7-2、介绍loopy寄存器（vram、tram）。

（从52分01秒到54分03秒）

当你阅读NES模拟器文档的时候，你肯定会遇到一位叫做loopy的朋友，这位朋友设计出了一种可以很方便的代表上述内存信息的内存结构。这个结构，跟我在前面展示的把“coarse x”和“coarse y”以及名称表比特位组合而成的12位地址是一样的。到目前为止，这个地址与你的所有其他地址都不相同。这是一个由PPU控制的地址，它负责让扫描线的位置与其他所有正在进行的操作相关联（This is an internal address maintained by the PPU that correlates the scanline position to while everything else that's going on.）。并且它几乎总是由PPU自己维护。事实上，这就是PPU在渲染图像时，你无法对它进行写入操作的原因。因为你会在无意中（inadvertently）改动该地址，这样会导致PPU无法分清需要读取的图形的源头（so the PPU would get confused with where it's reading its graphic sources from.）。我将会按照Loopy所提供的方法来设计，非常感谢Loopy设计出了这种，能够很便利地对PPU内部数据总线进行解读的方法。

不出所料，NESdev维基百科也推荐使用Loopy的方法（http:[http://wiki.nesdev.com/w/index.php/ppu_scrolling](http://wiki.nesdev.com/w/index.php/ppu_scrolling)，该网址已失效）（新网址：[https://www.nesdev.org/wiki/PPU_scrolling](https://www.nesdev.org/wiki/PPU_scrolling)），并且十分详细地（quite verbose）给我们解释了“Loopy寄存器”更新的时间和方式。

有2个寄存器被PPU操控，一个标记为v，这是在PPU递增的时候，可以根据需要获取数据的内部数据寄存器（one is labeled v, which is the internal data register that the PPU is incrementing as required to get the data.）。另一个标记为t，它可以受到用户影响，因此，当用户对PPU进行读写的时候，它就会被更新（updated）。

![](https://pic1.zhimg.com/v2-57ce2b6339bd9e9383212ec8f21135e4_r.jpg)

*图片（在维基百科中的Loopy寄存器的说明文档）*

通常，v寄存器的部分内容需要由t寄存器来更新。例如，当扫描枪到达扫描线末端时，促进扫描枪的焦点重置，并返回到已知位置。这两个寄存器把屏幕滚动信息与PPU地址组合起来，以便于访问内存中的正确地址（These registers combine the scrolling information as well as the PPU location, in order to access the right bytes of memory.）。

在本系列视频的第1集，我说过在NESdev网站上没有提供代码，但是，Loopy寄存器的介绍页面，是我所能找到一些可实现的虚拟代码的唯一页面（the Loopy register is the only place where I have actually found some implementable pseudo code.）。它到处都是用二进制位的形式编写的。

### 7-3、用vram寄存器存储递增信息。

（从54分03秒到54分49秒）

我的实施方案让这里（指coarse x和coarse y的设置）的代码有些冗长，所以我现在打算用这些Loopy寄存器替换临时创建的PPU地址变量（在olc2C02.h头文件中第109行代码：uint16_t ppu_address=0x0000;）。

看看寄存器的内部结构，它跟我之前描述的差不多。“coarse x”和“coarse y”变量会根据屏幕滚动的位置信息进行写入。给名称表分配两个比特位（NTx和NTy，用来确定需要使用的名称表是4个中的哪一个）。并且我们还创建了一个变量来存放“fine y”的位置。整个Loopy寄存器的结构只有15比特位，所以我用一个16位的字符来代表它。然后我会创建了两个Loopy寄存器，vram和tram。

![](https://pic4.zhimg.com/v2-b876ff2262700c4276aab9a053c4a343_r.jpg)

*图片（在olc2C02.h头文件中创建的Loopy寄存器的各标志位）*

现在唯一缺少的信息就是“fine x”的滚动信息。

```text
//在olc2C02.h头文件中添加以下代码：
行127  loopy_register vram_addr;
行128  loopy_retister_ tram_addr;
行130  uint8_t fine_x = 0x00;
行131  };
```

PPU地址现在已被vram地址有效替换。

```text
//在olc2C02.cpp文件中cpuRead函数的原代码：
行163  case 0x0007:  // PPU Data
行164   data = ppu_data_buffer;
行165   ppu_data_buffer = ppuRead( ppu_address );
行167   if( ppu_address >= 0x3f00 )  data = ppu_data_buffer;
行168   ppu_address += ( control.increment_mode ? 32:1 );
行169   break;
行170  }；
//修改后的代码：
行165   ppu_data_buffer = ppuRead( vram_addr.reg );
行167   if(vram_addr.reg >= 0x3f00 )  data = ppu_data_buffer;
行168   vram_addr.reg += ( control.increment_mode ? 32:1 );
```

在cpuRead函数中，除了自动递增功能之外（即上面的行168的代码），Loopy寄存器对所有其它操作都没有兴趣。

### 7-4、用tram寄存器存储名称表信息。

（从54分49秒到55分55秒）

但是，cpuWrite函数对于这些寄存器会产生很多影响。

控制寄存器（control.reg）有2个比特位，在4个名称表中，我们即将使用的名称表x、y坐标（NTx和NTy）就存放这里。所以，我打算把它们存放在tram地址变量中。

![](https://picx.zhimg.com/v2-5b12440098705fc7a18f8f172bb0f173_r.jpg)

*图片（在olc2C02.cpp中给cpuWrite函数添加的代码）*

我们从不直接对vram地址进行写入，而是写入到tram地址，所以我相应的修改了写入PPU地址寄存器的代码。不过，一旦对tram地址完成了16位地址的写入操作后，就会同步更新到vram地址中。

![](https://pic3.zhimg.com/v2-5cef27781940e14f6c2fc044c70ce50a_r.jpg)

*图片（在olc2C02.cpp中给cpuWrite函数添加的代码）*

引发上述复杂操作的寄存器就是滚动寄存器（scroll register）。而且跟上面一样，滚动寄存器也是分两次写入地址，每次成功写入都要进行部分屏蔽和右移（And again,this is written to in two halves and each write successively flip flops between the two halves of the address.）。

![](https://pic4.zhimg.com/v2-549a18e57142f341ac81f0377cf6141f_r.jpg)

*图片（在olc2C02.cpp中给cpuWrite函数添加的代码）*

写入滚动寄存器的数据设置在屏幕中，就是像素的偏移，所以，我们的“fine_x”偏移量的取值在0~7之间，即该数据最小的3个比特位。不过，我们也可以同步从该数据的相同字节中提取出“coarse_x”的地址。根据相同的方式，我们也可以获取到“fine_y”和“coarse_y”的值。

### 7-5、在时钟函数配置下一图块的加载时点。

（从55分55秒到57分23秒）

我现在要根据NESdev维基网站的时序表（timing table）来修改我的时钟函数（clock function）。我们已经创建了2个判断条件，用于设置垂直消隐标志位（vertical blank flag）。这些操作的目的都是针对所有可见扫描线的。对于特定扫描线上的一段周期，我们想要从中提取到图块ID、颜色属性信息、以及位图图案信息（And for a bunch of cycles on a particular scanline, we want to extract the tile ID, the attribute, and the bitmap patterns.）。当我们到达扫描线末端时，我想让Loopy寄存器的y值增加。

我会用一个switch case语句来实现图块中8个周期的循环功能，这8个周期是用来给PPU预加载接下来八个像素渲染所需的信息的。所以我需要创建一些变量来存储这些信息。

![](https://pic3.zhimg.com/v2-5ad12771f2e62d4fa89adb345ad481ee_r.jpg)

*图片（在olc2C02.cpp中给clock函数添加的代码）*

在olc2C02.h头文件中，我会添加4个变量：下一个背景图块ID变量（bg_next_tile_id）、下一个背景图块的调色板属性变量（bg_next_tile_attrib）、以及2个代表下一个八像素拼图的位图平面的8比特变量（two eight bit variables represent the plane of the pattern memory for the next eight pixels，即bg_next_tile_lsb和bg_next_tile_msb），请记住，它们是每个像素对应1比特。

![](https://pica.zhimg.com/v2-05adcbeab63988d8e5aed295a138fed2_r.jpg)

*图片（在olc2C02.h头文件中添加的代码）*

所以，要做的第一件事情就是读取图块ID（tile ID，case 0对应的代码）。

然后读取调色板属性ID（attribute ID，case 2对应的代码），别忘记了，虽然它是一个单独的字节，但它的某些比特位附带了额外的数据（that's a single byte that represents additional data split up into two bit patterns.）。所以为了提取出我们所需的最后2个比特位，还需要增加两步操作。

![](https://pic3.zhimg.com/v2-b06ea6c2737644d44f3cd9898f1030f2_r.jpg)

*图片（在olc2C02.cpp文件中给clock函数添加的代码）*

然后，我会提取“副位图平面”（least significant bit plane，LSB，即case 4对应的代码）的值，接着提取“主位图平面”（most significant bit plane，MSB，即case 6对应的代码）的值。你们可以看到，这两者的代码几乎一样，除了最后有个偏移量8，这点在前面讲过了。

![](https://pic4.zhimg.com/v2-bfddfc55310b1efb48e49e6f3d1670cb_r.jpg)

*图片（在olc2C02.cpp文件中给clock函数添加的代码）*




### 7-6、添加X轴递增、Y轴递增、X轴重置、Y轴重置代码。

（从57分23秒到60分44秒）

我知道你们看到这些代码之后，肯定会说“哇，太复杂了”。在随视频附带的源文件中，我花了很大的篇幅来准确解释每个阶段发生的事情。我把内存地址的比特位操作分解成了各个组成部分，并附带了详细说明。并且我已经在视频中对大部分的内容进行了讲解。但是当你们看到下图中这样的正式文件后，仍然会觉得没看懂，讲得不够细，这我可以理解。但我真的希望本视频的时长不要超过2个小时。

![](https://pic4.zhimg.com/v2-bcfaa180e3379c92e247dc789e36397b_r.jpg)

*图片（分享的源文件中附带了大量的详细注释）*

在帧时序结构图中（timing diagram），所有的周期都标注了红色，意味着我们将会对Loopy寄存器做一些额外的操作。并且还有4个基本函数：X轴方向的递增、Y轴方向的递增、X轴的坐标重置、Y轴的坐标重置。我准备把它们设置为clock()时钟函数中的匿名函数（lambda function）。

X轴方向递增函数（即auto IncrementScrollX= [&]()），只是简单的让vram的地址加1（vram_addr.corse_x ++;）。但是，当我们超出名称表边缘时，就把名称表的比特位取反（vram_addr.nametable_x=~vram_addr.nametable_x;），所以我们现在索引到了另一个名称表（So now we're indexing into the other name table）。

而这一行if代码要结合下一个视频才知道它的作用（if( mask.render_background || mask.render_sprites )）。但实际上，我们只有在渲染东西的时候才能做后面的事情。如果渲染器（renderer）被禁用，则后面的代码都不会起作用。而且渲染器是否启用，要受到掩码寄存器（mask register）的指定比特位的控制。

![](https://pica.zhimg.com/v2-eb820c34e737f8cab9c17e59a106e074_r.jpg)

*图片（在olc2C02.cpp文件中给clock函数添加的匿名函数之一）*

Y轴方向递增函数（即auto IncrementScrollY= [&]()）几乎也是一样的代码，只是我们增加的是Y轴地址。这是因为我们是在扫描线的基础上操作的，而扫描线只有1个像素的高度。

```text
//在olc2C02.cpp文件中给clock函数添加的y轴滚动函数
行365  auto IncrementScrollY = [&]()
行366  {
行367  if (mask.render_background || mask.render_sprites)
行368  {
行370   if (vram_addr.fine_y < 7)
行371   {
行372    vram_addr.fine_y++;
行373   }
行374   else
行375   {
行384   vram_addr.fine_y = 0;
行387   if (vram_addr.coarse_y == 29)
行388   {
行390    vram_addr.coarse_y = 0;
行392    vram_addr.nametable_y = ~vram_addr.nametable_y;
行393   }
行394   else if (vram_addr.coarse_y == 31)
行395   {
行398    vram_addr.coarse_y = 0;
行399    }
行400    else
行401    {
行404    vram_addr.coarse_y++;
行405    }
行406    }
行407   }
行408  };
```

然而，当我们沿着X轴方向推进的时候，我们每8个像素读取一次新的图块信息，所以X轴的地址是保持不变的。但在Y轴方向递增时，我们需要用到“fine_y”变量的值。如果“fine_y”大于8个像素，即一个图块的宽度，那么我们就增加“coarse_y”变量的值。并且像前面一样，如果我们在垂直方向超出了名称表的边缘，那么就把名称表的y比特位取反（vram_addr.nametable_y =~ vram_addr.nametable_y;），这样就能访问到另一个名称表，它们是成对出现的（It’s corresponding counterpart of the pair.）。 

重置X轴坐标地址函数，我称之为传送X轴地址函数（TransferAddressX），它就是一个把tram地址变量的x标志复制给vram地址变量的简单案例（Resetting the address, I’ve called it transferring the address, it’s simply a case of copying over the X components of the TRam address variable into the VRam address variable.）。重置Y轴坐标与之相似。

![](https://pic2.zhimg.com/v2-156d160abcb4a731175ac5c99d3629b3_r.jpg)

*图片（在olc2C02.cpp文件中给clock函数添加的重置X轴和重置Y轴函数）*

回到我们clock函数的switch case语句，一旦我们输出了8个像素，那么我们接下来马上要准备下一个图块了，所以我们会递增X轴（IncrementScrollX();）。

![](https://pica.zhimg.com/v2-5188136bec7c88ce3569734eed8f3a0c_r.jpg)

*图片（在olc2C02.cpp文件中给clock函数添加的case 7的代码）*

在我们把当前行的可见部分绘制完成后，我们就让Y轴递增（IncrementScrollY();）。但是，尽管我们递增了Y轴，我们的X轴坐标仍然是不对的，所以我们需要重置X轴坐标，让它回到扫描线的起点。并且，我们还需要在扫描线的不可见区域设置好Y轴的信息，为绘制下一帧图像做准备（And we’ll need to set a y components on the non-visible scanline ready for a new frame.）。奇怪的是，在第240号扫描线，系统什么都不做。

```text
//在olc2C02.cpp文件中给clock函数添加的代码：
行472   if (cycle == 256)
行473   {
行474    IncrementScrollY();
行475   }
行477   if (cycle == 257)
行478   {
行479    TransferAddressX();
行480   }
行482   if (scanline == -1 && cycle >= 280 && cycle < 305)
行483   {
行484    TransferAddressY();
行485   }
行486   }
行488   if (scanline == 240)
行489   {
行490    // Post Render Scanline - Do Nothing!
行491   }
行493   if (scanline == 241 && cycle == 1)
行494   {
行495    status.vertical_blank = 1;
行496    if (control.enable_nmi) 
行497     nmi = true;
```

我们快要完成了。我们能够精确地知道具体需要访问的背景图块是哪一块，因为我们有了当前所在名称表的信息，以及电子枪在当前屏幕上的位置。当然，现在所有的事情都只发生在8像素的图块中，因为我们已经从内存中读取了这些数据，所以这8像素需要进行缓冲，以便于在接下来的8个周期中进行渲染。

我们已经把信息加载好，并存储在这4个变量中（bg_next_tile_id、bg_next_tile_attrib、bg_next_tile_lsb、bg_next_tile_msb）。但是，现在我们需要看看另一个与PPU稍微同步进行的进程，它获取这些信息，并将其合成为正确的颜色和渲染地址，NES游戏机会使用移位寄存器（shift registers）来完成这项任务。




### 7-7、移位寄存器如何控制像素移动的。

（从60分44秒到63分18秒）

下面的示意图显示的是某个扫描线上的16个像素点。在渲染左边8个像素的同时，程序也会加载右边8个像素的信息，而这8像素的信息会加载到“16比特的移位寄存器（shift registers，总共有4个，分别是：LSB、MSB、PAL_LO、PAL_HI，每个都是16比特）”的低字节中。因此，在每个周期中，“移位寄存器（shift registers）”会把每个像素左移1格（视频中口误说成右移）。所以，当我们到达图块的边缘时，下一图块的8像素信息就都调整到了“16比特的移位寄存器（shift registers）”的高字节中。

![](https://pic1.zhimg.com/v2-37079509a9acf3ae63ccfe00c214296c_r.jpg)

*图片（移位寄存器的示意图）*

让我们以“像素位平面（pixel bit planes）”为例。当我开始渲染这个像素的时候（绿色格子的左起第一个），我可以从“移位寄存器（shift register）”的最高有效位（most significant bits，即下图蓝色圈，别忘记了，一个图块的颜色由2个位图平面组成，所以需要获取2个平面的最高位）获取所需的像素值。当我移动到下一个像素时（如下图，绿色格子的左起第二个），这2个寄存器都会左移1位。所以，再次渲染像素时，我又可以直接取最高有效位的像素值。

![](https://picx.zhimg.com/v2-2fd6c9adccf0eda4dc73d3a902c8b391_r.jpg)

*图片（当逐一渲染像素时，移位寄存器会不断左移）*

在某种程度上，当扫描线向右渲染的时候，“移位寄存器（shift register）”向左移动，所以它们会集合在一起（converge），在正确的位置给我们提供正确的像素值。不过，也有些东西会影响这个值，就是我们在滚动寄存器（scroll register）中设置的“fine_x”的地址。

目前为止，所有事情都发生在图块边缘，但是我们想要保证屏幕滚动的像素精度，所以不会一直使用最高有效位，而是选用由“fine_x”寄存器所选择的比特位（But we want pixel precision for our scrolling, so instead of choosing the most significant bit all of the time, we choose the bit chosen by the fine X register.）。

假设“fine_x”的值等于3，那么我们就不是选用最高有效位（即第0位），而是从最高有效位往右数3个数的比特位（即第4位），这实际上让我们的图块滚动了3个像素。当然，程序还是像正常一样，“移位寄存器（shift register）”向左移动1位，扫描线也会同步向右移动1个像素。

![](https://pic4.zhimg.com/v2-604ea53d80c05ee0686dcb352caf0537_r.jpg)

*图片（程序根据“fine_x”的值进行图像偏移）*

调色板虽然只有2个比特，但是它们适用于整行（The palette were only represented as two bits, but they apply to the whole row.）。所以，当我们给当前图块加载调色板属性信息的时候，我会让“移位寄存器（shift register）”中的所有比特位都设置成同一个值（见下图，调色板高字节的所有比特位都相等，调色板低字节也是都相等）。通过这种方式，我就可以用相同的机制同步选出“渲染最终像素颜色”所需要的全部信息（That way I can use exactly the same mechanism to choose all of the information I need to get my final pixel color.）。

![](https://pic4.zhimg.com/v2-c0797b89150b7b0e13dc0248fe5ab447_r.jpg)

*图片（根据fine_x的值3，一次性就可以从移位寄存器的对应位置取得所需的数据）*

当扫描线扫到“正在背景中渲染的像素”时，我们就会加载下一个图块的8像素的信息。并且，因为这些“移位寄存器（shift registers）”都是16比特的空间，所以低字节的数据也会向左移1位（下图橙色字体的数据）。所以我们最终得到了这个永不停止的1比特的信息流动，它在X轴方向上给了我们流畅的图像滚动。

![](https://picx.zhimg.com/v2-219028dd3427d9d7f104df1eab9cea0b_r.jpg)

*图片（当渲染1个像素的图像时，低字节的数值也会向左移动1比特）*




### 7-8、配置移位寄存器的取数，更新像素、调色板的功能。

（从63分18秒到64分09秒）

我会用16比特的变量来代表我的四个“移位寄存器（shifters）”。

![](https://picx.zhimg.com/v2-901f988c96a93f6ba91c378efd28a99b_r.jpg)

*图片（在olc2C02.h头文件中添加的4个移位寄存器）*

而在clock()函数中，我会增加另外两个匿名函数（lambda function）。

一个函数（autoLoadBackgroundShifters = [&]()）用于给渲染程序准备移位寄存器。对于像素（即pattern_lo、pattern_hi），我们会把整个8比特的数据加载到16比特寄存器的低字节中。对于调色板属性（即attrib_lo、attrib_hi），我们会把单独的二进制位取出来（即attrib & 0b01和attrib & 0b10），然后把它复制成8比特的值，这样就能确保我们的调色板属性与图块像素属性是同步的（So this ensures our palettes are in sync with our patterns.，即都是8比特大小）。

![](https://picx.zhimg.com/v2-28422270be627028fe46cc6ae1376577_r.jpg)

*图片（在olc2C02.cpp文件中给clock函数添加的移位寄存器取值代码）*

第二个匿名函数的作用就比较简单，只是更新移位寄存器，它会把四个移位寄存器都往左移1位（视频中口误说成右移），有2个是更新图块像素寄存器的（for the pixels，即pattern_lo和pattern_hi），另外2个更新调色板属性寄存器的（for the palettes，即attrib_lo和attrib_hi）。

![](https://pic2.zhimg.com/v2-e3e020b566ff0c29f5c8b96ae1e2007d_r.jpg)

*图片（在olc2C02.cpp文件中给clock函数添加的更新移位寄存器的代码）*

在每个可见的周期，我们都要更新移位寄存器（shifters）。当内部循环计数器在前8个像素中循环时，我们就要把后8个像素的信息字段加载到背景移位寄存器中（And when the internal cycle counter loops around eight pixels, we're going to load our background shifters with the next eight pixels words of information.）。

![](https://pic4.zhimg.com/v2-bafebf5f88d6afd51abdda8155d9c9ff_r.jpg)

*图片（在olc2C02.cpp文件中给clock函数的可见周期中添加移位寄存器更新和取数功能）*




### 7-9、组合出绘制背景的代码。

（从64分09秒到65分22秒）

现在我们有一个周期和扫描线在跟踪着图像框架，我们也知道了当前绘制到哪个像素点，也知道了使用的调色板和位图是哪个，那么是时候把它们组合起来绘制成图像了。考虑到我们在前面做了这么多复杂的准备工作，现在这项组合工作看起来就很简单了。

如果我在掩码寄存器中启用了绘制背景功能，那么我应该可以绘制背景了。我需要根据“fine_x”的值来决定选择移位寄存器的哪个比特位，目前，代码默认是选择最高有效位（bit_mux = 0x8000;），所以我现在改成“根据fine_x的值来选择”（bit_mux =0x8000 >> fine_x;）。一旦该变量（bit_mux）的比特位被移动了，我就用它在像素移位寄存器（即pattern_lo和pattern_hi）中提取出指定比特位的像素值，并分别存入两个像素位图平面变量（即p0_pixel和p1_pixel）。现在，让我们看看如何让程序与视频开头关联起来（And see how we're getting things going back to the start of the video now.）。根据同样的方法，我会从调色板属性移位寄存器中，提取到调色板的2个比特的信息。

![](https://pic1.zhimg.com/v2-5a39f82dd0ac226e3c529ec9c87b9f22_r.jpg)

*图片（在olc2C02.cpp文件中添加的组合代码）*

这意味着，之前用来产生雪花点的代码，现在我可以用GetColourFromPaletteRam()函数来代替，然后把刚才提取的调色板和像素信息传递进去。这似乎花费了很长的时间，现在我们定义好了函数，可以根据实际选择在屏幕上绘制的最终颜色。

```text
//在olc2C02.cpp文件中的原代码：
行552  //Fake some noise for now
行553  sprScreen.SetPixel( cycle – 1, scanline, palScreen[ ( rand() % 2 ) ? 0x3F : 0x30 ] );
//修改后的代码
行553  sprScreen.SetPixel( cycle – 1, scanline, GetColourFromPaletteRam( bg_palette, bg_pixel ) );
```




## 8、再次测试屏幕渲染情况。

（从65分22秒到66分32秒）

好的，现在我们再来看看效果，启动模拟器。现在，我们看到这次《大金刚》渲染出了正确的颜色了。所以，之前程序是用了整个调色板来渲染，而现在它是根据最终输出的属性区域选择调色板的（So before it was rendered with a whole palette, but now it's choosing the palette depending on the attribute regions of the final output.）。而且，我们可以看到屏幕上的大金刚在兴奋的跳舞，当前它没有别的事做了。有趣的是，它只是背景图像的一部分。

![](https://pic2.zhimg.com/v2-375a04f5994cd040cf87024aa121025b_r.jpg)

*图片（模拟器成功绘制出了《大金刚》游戏的画面）*

但是《超级马里奥兄弟》运行起来就有点糟糕，有一个原因造成了这种问题，我们会在下一个视频了解更多关于它的情况。

![](https://pica.zhimg.com/v2-39b0865db1fb71e0a7a822d9a4b43d3e_r.jpg)

*图片（《超级玛丽》的标题画面显示不全）*

在本阶段，很难找到支持屏幕滚动的游戏标题画面，因为我们只配置了0号映射器（mapper 0），它是一款很简单的映射器，所以大多数支持它的游戏都是单屏幕的。正如我们刚刚在《超级玛丽》中看到的，在游戏标题画面演示滚动之前还需要启用一些其他功能。

但这款《敲冰块》游戏的标题画面也正常滚动了，所以，在屏幕上我们可以看到它的垂直滚动都很顺畅。有趣的是，在它的演示画面中，我们可以看到背景的名称表中某些部分消失了，这是因为雪山兄弟把他头上的冰块敲掉了（只是我们还没有实现前景功能，所以看不到人）。

![](https://pic4.zhimg.com/v2-14bd4c2e261ca14f8f455a6a511b5f29_r.jpg)

*图片（《敲冰块》游戏的标题演示画面）*

还有一款是古老的《kung fu》游戏，看到它不会动我觉得挺高兴的，因为它是一款糟糕的游戏（图片略）。




## 9、结束语。

（从66分32秒到67分08秒）

所以，这是一个很长、很复杂的视频，也是目前为止本系列视频中最长、最复杂的一集。额，接下来的视频，我们会学习精灵（即前景）。然后再学习声音，声音应该会比本视频简单很多。

如果你喜欢本视频，请给我点个赞，并且考虑关注我的主页。再次感谢所有关注我的朋友和加入了油管频道的朋友，从来没想过竟然会有这么多朋友关注我，嗯，欢迎大家经常来Discord Servers网站交流学习。

如果你有问题的话，可以在GitHub网站的onelonecoder页面下找到源代码。我们下次再见，保重。




## 10、可供参考资料：

1、油管大神在GitHub上的主页，[https://github.com/OneLoneCoder](https://github.com/OneLoneCoder)。

2、源代码下载链接，[https://github.com/OneLoneCoder/olcNES](https://github.com/OneLoneCoder/olcNES)。

3、哔哩哔哩上的系列教学视频，[https://www.bilibili.com/video/BV1Uv4y1v7T9?p=5](https://www.bilibili.com/video/BV1Uv4y1v7T9?p=5)。

4、作者“rand_cs”的《童年神机小霸王原理(三) PPU 》，[https://mp.weixin.qq.com/s/lJPAY3c2N5AgbMzA7p7-Uw](https://mp.weixin.qq.com/s/lJPAY3c2N5AgbMzA7p7-Uw)。

5、作者“真把你打了”的《浅谈FC的图像显示技术》，[https://zhuanlan.zhihu.com/p/43546104](https://zhuanlan.zhihu.com/p/43546104)。

6、NESdev Wiki网的《NTSC PPU Frame Timing》帧时序结构图，[https://www.nesdev.org/wiki/File:Ppu.svg](https://www.nesdev.org/wiki/File:Ppu.svg)。

7、NESdev Wiki网的《loopy寄存器说明文档》，[https://www.nesdev.org/wiki/PPU_scrolling](https://www.nesdev.org/wiki/PPU_scrolling)。

8、NESdev Wiki网，[https://www.nesdev.org/wiki/Nesdev_Wiki](https://www.nesdev.org/wiki/Nesdev_Wiki)。

## 11、专用名词：

1、PPU，全称Picture Processing Unit，图像处理器、或图形处理器。

2、BUS，总线，是计算机各种功能部件之间传送信息的公共通信干线，它是由导线组成的传输线束， 按照计算机所传输的信息种类，计算机的总线可以划分为数据总线、地址总线和控制总线，分别用来传输数据、数据地址和控制信号。

3、RAM，全称random access memory，随机存储器，也叫内存，是与CPU直接交换数据的内部存储器。它可以随时读写（刷新时除外），而且速度很快，通常作为操作系统或其他正在运行中的程序的临时数据存储介质。

4、ROM，全称read only memory，只读存储器，也叫程序存储器。在本教程中，指的是存储游戏代码的空间。

5、比特bit：1位。字节Byte：8位。字word：2字节，相当于2个Byte，16位。双字doubleword：4字节，32位。八字节：8字节，64位。1KB：1024字节。（0x0000就等于1个字节，0x0000~0x000F等于16个字节，0x0000~0x00FF等于256个字节）。

6、图像存储器，全称pattern memory，也叫CHR、拼图库、pattern table图案表，由2个拼图构成。这个图像存储器里面存储的是图形（graphics）和精灵（sprites）。跟CPU的内存类似，图像文件地址也是从0x0000到0x1FFF。每个游戏都会有自己的特定的图像文件，例如魂斗罗就有红色和蓝色的主角，但肯定不是马里奥。

![](https://pic1.zhimg.com/v2-2e31581f4754112ffd2b1c4dbb8d4496_r.jpg)

*图片（超级马里奥的其中一个拼图）*

![](https://pic4.zhimg.com/v2-bd181db277b0dfa613ec1f4cdc4e5755_r.jpg)

*图片（魂斗罗的2个拼图）*

7、名称表内存，全称name tables，或V-RAM，是PPU自己的内存。它专门用于存放名称，这些名称就是以二维数组形式存储的“展示在游戏背景中的图像的ID号”。

8、调色板，全称palettes。它负责存储颜色的索引信息，这些信息描述的是“当精灵和背景结合的时候，屏幕上所应该显示的颜色”。总调色板是游戏可以使用的全部颜色的集合，但是屏幕上每个图块只能使用1个子调色板，所以每个图块最多只有4种颜色。“子调色板”存放的是“总调色板”中颜色的索引号。

![](https://picx.zhimg.com/v2-962d088e5b9b36e43fd65ba9793968f9_r.jpg)

*图片（NES游戏的总调色板）*

![](https://pic3.zhimg.com/v2-f772be76a4f21b1e17a3bd3e8edb4d2e_r.jpg)

*图片（8个子调色板，每个只有4种颜色）*

9、VRAM，全称Video Random Access Memory，显存，帧存储器，刷新存储器。显卡上的随机存取存储器，是一种双端口内存，允许在同一时间被所有硬件（包括中央处理器、显示芯片等）访问。

10、Sprite，精灵，活动块，指的是游戏显示屏上所显示的漂浮于背景之上的、可以独立行动的物体，例如玩家角色、NPC、敌军。

11、Bitmap，位图，又称栅格图或点阵图，是使用像素阵列(Pixel-array/Dot-matrix点阵)来表示的图像。

12、硬编码，hard-coding，相当于刻录，是指将常量、数值或配置直接嵌入到源代码中的做法。例如，在代码中直接使用数字、字符串或其他常量，而不将它们抽象成变量或配置文件。

13、垂直消隐，vertical blank，屏幕在绘制图像时，每一帧图像都是由扫描枪把像素一个个打出来的，它总是从图像的左上角开始，水平向右扫描，同时扫描点也以较慢的速率向下移动。当扫描点到达图像右侧边缘时，扫描点快速返回左侧，重新开始在第1行的起点下面进行第2行扫描，行与行之间的返回过程称为水平消隐。一幅完整的图像扫描信号，由水平消隐间隔分开的行信号序列构成，称为一帧。扫描点扫描完一帧后，要从图像的右下角返回到图像的左上角，开始新一帧的扫描，这一时间间隔，叫做垂直消隐，也称场消隐（VBlank）。

14、Attribute Memory，属性内存，属性存储器，存放着“子调色板”的索引号。