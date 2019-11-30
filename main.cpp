/*
 * Copyright (C) 2019  明心  <imleizhang@qq.com>
 * All rights reserved.
 *
 * This program is an open-source software; and it is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * This program is not a free software; so you can not redistribute it and/or
 * modify it without my authorization. If you need use it for any
 * commercial purpose, you should first get commercial authorization from
 * me, otherwise there will be your's credit and legal risks.
 *
 */

#include <gObject.h>
#include <string>

using namespace Giveda;
using namespace std;

/**
 * @mainpage 基于c++11实现的信号和槽
 * @author 明心(imleizhang\@qq.com)
 * @version 1.0.0
 * @date 2018-12-27
 */

/**
 * @class Receiver
 * @brief Receiver类负责接收信号；并进行业务处理。
 */
class Receiver : public GObject
{
public:
    Receiver ( )
    {}

public slots:
    /**
     * @brief 接收sender发射的selected信号、并进行业务处理
     *
     * @param const string& selected信号传递过来的参数
     * @param int    selected信号传递过来的参数
     * @return void
     */
    void  slotSelected ( const string& str, int idx )
    {
        printf ( "Receiver::slotSelected is called. " );
        printf ( "param=[%s] [%d]\n", str.c_str(), idx );
    }

    void slotClicked()
    {
        printf ( "Receiver::slotClicked is called\n" );
    }
};

/**
 * @class Sender
 * @brief Sender类负责定义信号；并负责在需要时，发射信号。
 */
class Sender  : public GObject
{
public:
    /**
     * 定义一个名称为selected的信号；该信号接收两个参数，参数类型分别为const string&和int
     */
    GSignal<void ( const string&, int ) > selected;

    /**
     * 定义一个名称为clicked的信号；该信号不接收任何参数
     */
    GSignal<void ( void ) > clicked;

public:
    /**
     * @brief 发射信号
     *
     * @return void
     */
    void notify()
    {
        string a ( "giveda.com" );
        selected ( a, 3 );
        clicked();
    }
};


int main ( int /*argc*/, char** /*argv*/ )
{
    Sender *s = new Sender;
    s->notify();
    printf ( "before connect\n" );

    Receiver *r = new Receiver;
    GObject::connect ( s, s->selected, r, &Receiver::slotSelected );
    GObject::connect ( s, s->clicked, r, &Receiver::slotClicked );
    printf ( "after connect\n" );
    s->notify();

    GObject::disconnect ( s, s->selected, r, &Receiver::slotSelected );
    GObject::disconnect ( s, s->clicked, r, &Receiver::slotClicked );
    printf ( "after disconnect\n" );
    s->notify();

    printf ( "re-connected, but delete receiver\n" );
    GObject::connect ( s, s->selected, r, &Receiver::slotSelected );
    GObject::connect ( s, s->clicked, r, &Receiver::slotClicked );
    delete r;
    s->notify();

    delete s;

    return 0;
}

