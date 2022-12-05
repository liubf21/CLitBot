#include<iostream>
#include<cstring>
#include<fstream>

using namespace std;
const int MAX_OPS=50,MAX_PROCS=10,MAX_ROW=10,MAX_COL=10;
const int MAX_LIT=50,MAX_PATH_LEN=50;
// 各模块公用的全局变量
// 位置类型，可用来表达机器人或灯所在位置
struct Position {
 int x, y; // x 表示列号，y 表示行号
};
// 方向枚举类型，可用来表达机器人朝向，只有四种取值
enum Direction {
 UP, DOWN, LEFT, RIGHT
};
// 机器人类型
struct Robot {
 Position pos; // 机器人位置
 Direction dir; // 机器人朝向
};
// 灯类型
struct Light {
 Position pos; // 灯位置
 bool lighten; // 是否被点亮
};
// 单元格类型
struct Cell {
 int height; // 高度
 int light_id=-1; // 灯标识，-1表示该单元格上没有灯(1表示有灯，0表示被点亮)
 bool robot; // true/false分别表示机器人在/不在该单元格上
};
// 指令类型
enum OpType {
 TL, TR, MOV, JMP, LIT, CALL
};
// TL为左转，TR为右转，MOV为向前行走，JMP为跳跃，LIT为点亮灯；
// 使用CALL表示调用MAIN，CALL + 1表示调用P1，以此类推。
// 过程类型
struct Proc {
 OpType ops[MAX_OPS]; // 指令记录，MAX_OPS为合理常数
 int count; // 有效指令数
};
// 指令序列类型
struct OpSeq {
 // 过程记录，MAX_PROCS为合理常数
 // procs[0]为MAIN过程，procs[1]为P1过程，以此类推
 Proc procs[MAX_PROCS];
 int count; // 有效过程数
};
// 地图状态类型
struct Map {
 // 单元格组成二维数组，MAX_ROW、MAX_COL为合理常数
 Cell cells[MAX_ROW][MAX_COL];
 int row, col; // 有效行数、有效列数
 // 灯记录，MAX_LIT为合理常数
 Light lights[MAX_LIT];
 int num_lights; // 有效灯数
 // 地图上同时只有一个机器人
 Robot robot;
 // 每个过程的指令数限制
 int op_limit[MAX_PROCS];
 int num_procs; // 最多允许的过程数(自己添加的变量)
};
// 游戏状态类型
struct Game {
 char map_name[MAX_PATH_LEN]; // 当前地图的文件路径名
 Map map_init; // 地图初始状态
 Map map_run; // 指令执行过程中的地图状态
 // 自动保存的文件路径名，MAX_PATH_LEN为合理常数
 char save_path[MAX_PATH_LEN];
 int auto_save_id; // 自动保存标识(为1时开启)
 int limit=100; // 执行指令上限（用来避免无限递归）
};
Game game; // 全局唯一的Game变量

// 执行模块
void robot_run(){
    cout<<"run!";
}

// 交互模块
OpType trans(char op[]){
    if(!strcmp(op,"TL")){
        return TL;
    }else if(!strcmp(op,"TR")){
        return TR;
    }else if(!strcmp(op,"MOV")){
        return MOV;
    }else if(!strcmp(op,"JMP")){
        return JMP;
    }else if(!strcmp(op,"LIT")){
        return LIT;
    }else if(!strcmp(op,"CALL")){
        return CALL;
    }else if(op[0]=='P')
    {
        return OpType(CALL+op[1]-'0');
    }else return OpType(100); // 错误指令
}
// 显示当前地图(测试时显示初始地图)
void printMap(int k = 0)
{
    Map tmpmap = game.map_init;
    if (k)
    {
        tmpmap = game.map_run;
    }
    for (int i = 0; i < tmpmap.row; i++)
    {
        for (int j = 0; j < tmpmap.col; j++)
        {
            // 高度不为0才输出
            if (tmpmap.cells[i][j].height)
            {
                cout << "\e[";
                // 判断格子是否有机器人，是否有灯（有灯判断是否被点亮
                if (tmpmap.cells[i][j].robot)
                    cout << "91;";
                else
                    cout << "92;";
                if (tmpmap.cells[i][j].light_id == 1)
                    cout << "104;";
                else if (tmpmap.cells[i][j].light_id == 0)
                    cout << "103;";
                else
                    cout << "100;";
                cout << "1m" << tmpmap.cells[i][j].height;
            }
            else
                cout << "\e[0m ";
        }
        cout << "\e[0m" << endl;
    }
    cout << "Robot is facing ";
    switch (tmpmap.robot.dir)
    {
    case UP:
        cout << "up." << endl;
        break;
    case DOWN:
        cout << "down." << endl;
        break;
    case LEFT:
        cout << "left." << endl;
        break;
    case RIGHT:
        cout << "right." << endl;
        break;
    }
}
int main(){

    char cmd[20], op_[10];
    char path[MAX_PATH_LEN];
    OpSeq opseq; // 当前读出的指令
    // Robot bot;
    while(1)
    {
        cin>>cmd;
        // 判断命令
        if(!strcmp(cmd,"LOAD")){
            cin>>game.map_name;
            // 读取文件信息
            ifstream fin(game.map_name);
            if (!fin.fail())
            {

                while (!fin.eof()) // 文件存在且未读完
                {
                    fin >> game.map_init.row >> game.map_init.col 
                    >> game.map_init.num_lights >>game.map_init.num_procs;
                    // 地图从下标为0开始
                    for(int i=0;i<game.map_init.row;i++)
                    {
                        for(int j=0;j<game.map_init.col;j++)
                        {
                            fin>>game.map_init.cells[i][j].height;
                        }
                    }
                    int x,y;
                    for(int i=0;i<game.map_init.num_lights;i++)
                    {
                        fin>>x>>y;
                        game.map_init.cells[y][x].light_id=1;
                    }
                    for(int i=0;i<game.map_init.num_procs;i++)
                    {
                        fin>>game.map_init.op_limit[i];
                    }
                    fin>>game.map_init.robot.pos.x>>game.map_init.robot.pos.y;
                    int tmp;
                    fin>>tmp;
                    switch (tmp)
                    {
                    case 0:
                        game.map_init.robot.dir = UP;
                        break;
                    case 1:
                        game.map_init.robot.dir = DOWN;
                        break;
                    case 2:
                        game.map_init.robot.dir = LEFT;
                        break;
                    case 3:
                        game.map_init.robot.dir = RIGHT;
                        break;
                    }
                }
            }
            else cout<<"File failed!"<<endl;
            fin.close();
        }else if(!strcmp(cmd,"AUTOSAVE")){
            cin>>game.save_path;
            if(!strcmp(game.save_path,"OFF")){
                game.auto_save_id=0;
            }else game.auto_save_id=1;
        }else if(!strcmp(cmd,"LIMIT")){
            cin>>game.limit;
        }else if(!strcmp(cmd,"STATUS")){
            cout<<"Map Name: "<<game.map_name<<endl;
            cout<<"Autosave: "<<game.save_path<<endl;
            cout<<"Step Limit: "<<game.limit<<endl;
            printMap(); // 显示测试地图(传入1时显示当前地图)
            cout<<"Proc Limit: [";
            for(int i=0;i<game.map_init.num_procs-1;i++)cout<<game.map_init.op_limit[i]<<", ";
            cout<<game.map_init.op_limit[game.map_init.num_procs-1]<<"]"<<endl;
        }else if(!strcmp(cmd,"OP")){
            // 新建指令序列
            cin>>path;
            int n,m,wrong=0;
            cin>>n;
            if(n>game.map_init.num_procs)wrong=1;
            ofstream fout(path);
            fout<<n<<endl;
            for(int i=0;i<n;i++)
            {
                cin>>m;
                if(m>game.map_init.op_limit[i])wrong=1;
                fout<<m<<" ";
                // 指令的输入
                for(int j=0;j<m;j++)
                {
                    cin>>op_;
                    fout<<op_<<" ";
                }
                fout<<endl;
            }
            fout.close();
            if(wrong)cout<<"Limit Exceeded!"<<endl;
        }else if(!strcmp(cmd,"RUN")){
            // 执行指令序列
            int tmp=0; // 执行指令的总个数必须在执行中判断（可能有递归）
            cin>>path;
            ifstream fin(path);
            if (!fin.fail()){
                fin>>opseq.count;
                for(int i=0;i<opseq.count;i++)
                {
                    fin>>opseq.procs[i].count;
                    for(int j=0;j<opseq.procs[i].count;j++)
                    {
                        // 读入指令
                        fin>>op_;
                        opseq.procs[i].ops[j]=trans(op_);
                        if(opseq.procs[i].ops[i]!=100)tmp++;
                        // opseq.procs[i].ops[j]=OpType(CALL+1);
                    }
                }
                robot_run();
                cout<<"RUN "<<path<<", result: "<<endl;
                int unlight;
                for(int i=0;i<game.map_run.num_lights;i++)
                {
                    if(!game.map_run.lights[i].lighten)unlight=1;
                }
                if(unlight)cout<<"unligthed"<<endl;
                else cout<<"LIGHT"<<endl;
                cout<<"Step(s) used: "<<tmp<<endl;
                printMap(1);
            }else cout<<"File failed!"<<endl;
            fin.close();

        }else if(!strcmp(cmd,"EXIT")){
            // 结束程序
            cout<<"Exit Game."<<endl;
            break;
        }else{
            cout<<"Please enter correct command!!!"<<endl;
        }
    }

    return 0;
}