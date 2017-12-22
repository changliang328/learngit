.
├── bf_ca
│   ├── hal			//所有的业务逻辑都在这，fingerprintd要实现的功能，还有测试工具，所有调用的入口都是这里．加载配置也在这里
│   └── tac			//实现CA向TA发送数据，buf．TA收到buf以后，解析成不通的命令
│       ├── beanpod
│       │   └── include
│       ├── isee
│       │   └── include
│       ├── qsee
│       │   ├── include
│       │   └── libs
│       │       ├── arm64-v8a
│       │       └── armeabi-v7a
│       ├── ree
│       │   └── include
│       ├── rsee
│       │   ├── include
│       │   └── libs
│       │       ├── arm64-v8a
│       │       └── armeabi-v7a
│       ├── sprtrusty
│       │   ├── include
│       │   └── libs
│       │       ├── arm64-v8a
│       │       └── armeabi-v7a
│       ├── trustkernel
│       │   ├── include
│       │   └── libs
│       │       ├── arm64-v8a
│       │       └── armeabi-v7a
│       └── trustonic
│           ├── include
│           └── libs
│               ├── arm64-v8a
│               └── armeabi-v7a
├── bf_ta
│   ├── chips		//芯片模块，主要是芯片相关的初始化参数，还有操作
│   │   └── obj
│   ├── core		//核心模块，包含	bl_data;pbf_algo;pbfimage;pbfTemplateMgr;一开始就会分配好内存了
│   │   └── obj
│   └── platform	//不同的TA编译目录．实现ta_plat.c，基本spi读写，文件读写等操作
│       ├── beanpod
│       │   ├── fp_server
│       │   │   ├── include
│       │   │   ├── libs
│       │   │   ├── obj
│       │   │   │   └── src
│       │   │   └── src
│       │   └── fp_ta_lib
│       │       ├── include
│       │       ├── obj
│       │       │   └── src
│       │       └── src
│       ├── include
│       ├── isee
│       └── ree
├── include
│   ├── cutils
│   ├── hardware
│   ├── hardware_legacy
│   ├── log
│   └── system
└── libs
    ├── arm64-v8a
    ├── armeabi
    └── armeabi-v7a

