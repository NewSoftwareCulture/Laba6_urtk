#include <DOS.H>
#include <conio.h>
#include <stdio.h>
#include <iostream.h>

enum key
{
    key_enter = 13
  , key_esc = 27
  , key_space = 32
  , key_1 = 49
  , key_2 = 50
  , key_3 = 51
};

enum Way
{
    v_down = 0x20   //SN[7]
  , v_up = 0x10     //SN[6]
  , h_left = 0x08   //SN[4]
  , h_right = 0x04  //SN[3]
  , r_right = 0x02  //SN[1]
  , r_left = 0x01   //SN[0]
};

unsigned char inputD() 
{
    unsigned char data = 0;
    outportb(0x37A, 0x2E);
    outportb(0x378, 0xFF);
    outportb(0x37A, 0x2f);
    delay(1);
    data = inportb(0x378);
    outport(0x37A, 0x04);
    return data;
}
void writeRC(char data)
{
    outportb(0x37A, 0x04);
    outportb(0x378, data);
    outportb(0x37A, 0x00);
    delay(1);
    outportb(0x37A, 0x04);
}

void writeRD(char data)
{
    outportb(0x37A, 0x0E);
    outportb(0x378, data);
    outportb(0x37A, 0x0A);
    delay(1);
    outportb(0x37A, 0x0E);
}

void start(char RC, char DRV)
{
    writeRC(RC);
    writeRD(0x80);
    writeRD(DRV);
}

void stop(char RC)
{
    writeRC(RC);
    writeRD(0x00);
    writeRD(0x00);
}

void readRD(unsigned char& SNS0, unsigned char& SNS1, unsigned char& SNS2)
{
    writeRC(0x13);
    SNS0 = inputD();
    SNS1 = inputD();
    SNS2 = inputD();
}

void interface(int vertical, int horizontal, int rotation)
{
    unsigned char SNS0, SNS1, SNS2;
    readRD(SNS0, SNS1, SNS2);
    int SNS[3] = { SNS0, SNS1, SNS2 };
    std::cout << "SNS0: " << SNS[0] << " SNS1: " << SNS[1] << " SNS2: " << SNS[2] << std::endl;
    int SN[18];
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 6; j++)
        {
            SN[i*6 + j] = (SNS[i] + 1) % 2;
            std::cout << " SN[" << i*6 + j << "]:" << SN[i*6 + j];
        }
    cout << "\n       M0 M1 M2 M3 M4 M5";
    for (int i = 1; i < 4; i++)
    {
        switch (i)
        {
        case 1: cout << "Start"; break;
        case 2: cout << "End  "; break;
        case 3: cout << "Move "; break;
        }
        for (int j = 0; j < 6; j++)
        {
            cout << " ";
            if (j != 0)
                cout << " ";
            cout << SN[((i - 1) + (3 * j))];
        }
        cout << endl;
    }
    cout << "      " << vertical << " " << horizontal << " " << rotation;
}

void move(char direction) //Прохождение 1 метки
{
    start(0x11, direction); delay(20); stop(direction);
}

void boost_move(char direction, int position, int& vertical, int& horizontal, int& rotation)
{
    unsigned int pos = position;
    for (int i = 0; i < pos; i++)
    {
        move(direction);
        if(getch() == key_esc) return 0;
        switch(direction)
        {
            case h_left: { horizontal++; break; }
            case h_right: { horizontal--; break; }
            case v_up: { vertical++; break; }
            case v_down: { vertical--; break; }
            case r_right: { rotation++; break; }
            case r_left: { rotation--; break; }
        }
        clrscr();
        cout << "            <<Positional control>> \n\n";
        interface(vertical, horizontal, rotation);
        if(SN[0] == 1 || SN[1] == 1 || SN[3] == 1 || SN[4] == 1 || SN[6] == 1 || SN[7] == 1)
            return 0;
    }
    stop(0x11);
}

void direction_reverce(char& direction)
{
    switch(direction)
    {
        case hor:
        {
            if (hor == h_right) { stop(h_right); delay(500); start(0x11, h_left); direction = h_left; }
            if (hor == h_left) { stop(h_left); delay(500); start(0x11, h_right); direction = h_right; }
        }
        case vert:
        {
            if (vert == v_up) { stop(v_up); delay(500); start(0x11, v_down); direction = v_down; }
            if (vert == v_down) { stop(v_down); delay(500); start(0x11, v_up); direction = v_up; }
        }
        case rot:
        {
            if (rot == r_right) { stop(r_right); delay(500); start(0x11, r_left); direction = r_left; }
            if (rot == r_left) { stop(r_left); delay(500); start(0x11, r_right); direction = r_right; }
        }
    }
}

Way to_horizontal(int position)
{
    if(position > 0) return h_left;
    if(position < 0) return h_right;
}

Way to_vertical(int position)
{
    if(position > 0) return v_up;
    if(position < 0) return v_down;
}

Way to_rotation(int position)
{
    if(position > 0) return r_right;
    if(position < 0) return r_left;
}

void ciclic_control_start(char hor, char vert, char rot)
{
    delay(1);
    start(0x11, hor);
    start(0x11, vert);
    start(0x11, rot);
}

void ciclic_control_stop(char hor, char vert, char rot)
{
    delay(1);
    stop(hor);
    stop(vert);
    stop(rot);
}

void to_start()
{
    int i = 0;
    if(SN[7] != 1) start(0x11, v_down);
    if(SN[4] != 1) start(0x11, h_left);
    if(SN[1] != 1) start(0x11, r_right);
    while(i==0)
    {
        if(SN[1] == 1 && SN[4] == 1 && SN[7] == 1) break;
        if(SN[1] == 1) stop(r_right);
        if(SN[4] == 1) stop(h_left);
        if(SN[7] == 1) stop(v_down);
    }
}

int main()
{
    int i = 0, vertical = 0, horizontal = 0, rotation = 0, position;
    char c_vertical, c_horizontal, c_rotation;
    char control;
    while(i == 0)
    {
        cout << "Press 'Q' for <<Cyclic control>> or 'W' for <<Positional control>>.\n";
        cin >> control;
        clrscr();
        if(control == "Q")
        {
            ciclic_control_start(v_up, h_left, r_right);
            c_vertical = v_up;
            c_horizontal = h_left;
            c_rotation = r_right;
            int pause_counter = 0;
            while(i == 0)
            {
                clrscr();
                cout << "            <<Ciclic control>> \n\n";
                interface(vertical++, horizontal++, rotation++);
                if(SN[0] == 1 || SN[1] == 1)
                    direction_reverce(c_rotation);
                if(SN[3] == 1 || SN[4] == 1)
                    direction_reverce(c_horizontal);
                if(SN[6] == 1 || SN[7] == 1)
                    direction_reverce(c_vertical);
                switch(getch())
                {
                    case key_enter:
                    {
                        direction_reverce(c_vertical); direction_reverce(c_horizontal); direction_reverce(c_rotation);
                        break;
                    }
                    case key_space:
                    {
                        if(pause_counter%2 == 0)
                            ciclic_control_stop(c_vertical, c_horizontal, c_rotation);
                        if(pause_counter%2 == 1)
                           ciclic_control_start(c_vertical, c_horizontal, c_rotation);
                        break;
                    }
                    case key_esc: return 0;
                }
            }
        }
        if(control == "W")
        {
            to_start();
            while(i == 0)
            {
                cout << "            <<Positional control>> \n\nEnter the degree of freedom.\n";
                switch (getch())
                {
                    case key_1: { cout << "Enter a coordinate.\n"; cin >> position;
                    boost_move(to_horizontal(position), position, vertical, horizontal, rotation); break; }
                    case key_2: { cout << "Enter a coordinate.\n"; cin >> position;
                    boost_move(to_vertical(position), position, vertical, horizontal, rotation); break; }
                    case key_3: { cout << "Enter a coordinate.\n"; cin >> position;
                    boost_move(to_rotation(position), position, vertical, horizontal, rotation); break; }
                    case key_esc: return 0;
                }
            }
        }
    }
    return 0;
}
