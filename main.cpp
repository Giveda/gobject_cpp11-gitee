/*
 * Copyright (C) 2019  明心  <imleizhang@qq.com>
 * All rights reserved.
 *
 * This program is an open-source software; and it is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * This program is not a free software; so you can not redistribute it and/or
 * modify it without my authorization. If you only need use it for personal
 * study purpose(no redistribution, and without any  commercial behavior),
 * you should accept and follow the GNU AGPL v3 license, otherwise there
 * will be your's credit and legal risks.  And if you need use it for any
 * commercial purpose, you should first get commercial authorization from
 * me, otherwise there will be your's credit and legal risks.
 *
 */

#include <gObject.h>
#include <string>

/**
 * @mainpage 基于c++11实现的信号和槽
 * @author 明心(imleizhang\@qq.com)
 * @version 1.0.0
 * @date 2018-12-27
 */


/**
 * @class Receiver
 * @brief Receiver类负责接收信号；并进行业务处理。
 * 通常情况下，一个文件应当只包含一个类，应当由一个名为receiver.cpp/receiver.h的文件来放置Receiver类。
 * receiver.cpp/receiver.h不需要include(依赖)Sender.h。
 * 请看main函数中的测试代码，测试代码解释了为什么Receiver需要继承于GObject。
 *
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
        printf ( "param=[%s, %d]\n", str.c_str(), idx );
    }
};

/**
 * @class Sender
 * @brief Sender类负责定义信号；并负责在需要时，发射信号。
 * 通常情况下，一个文件应当只包含一个类，应当由一个名为sender.cpp/sender.h的文件来放置Sender类。
 * sender.cpp/sender.h不需要include(依赖)receiver.h。
 * 请看main函数中的测试代码，测试代码解释了为什么Sender和Receiver需要继承于GObject。
 */
class Sender  : public GObject
{
public:
    /**
     * 定义一个名称为selected的信号；该信号接收两个参数，参数类型分别为const string&和int
     */
    GSignal<const string&, int> selected;

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

        a = "Hello, Giveda!";
        selected.emit ( a, 6 );
    }
};


/**
 * @brief 通常情况下，main函数应当放置在一个单独文件比如名为main.cpp。
 * main.cpp需要include(依赖)Sender.h和Receiver.h。
 *
 */
int main ( int /*argc*/, char** /*argv*/ )
{
    Sender *s = new Sender;
    s->notify();
    printf ( "before connect\n" );

    Receiver *r = new Receiver;
    GObject::connect ( s, s->selected, r, &Receiver::slotSelected );
    printf ( "after connect\n" );
    s->notify();

    GObject::disconnect ( s, s->selected, r, &Receiver::slotSelected );
    printf ( "after disconnect\n" );
    s->notify();

    //请看如下这段代码，这里解释了为什么Sender和Receiver需要继承于GObject。
    //Receiver对象被销毁后，Sender对象与Receiver对象之间的依赖会自动被解除。
    //事实上，如果你根本不在意Receiver被销毁后的情形，那么你当然可以不用继承于GObject。
    printf ( "re-connected, but delete receiver\n" );
    GObject::connect ( s, s->selected, r, &Receiver::slotSelected );
    delete r;
    s->notify();

    return 0;
}
