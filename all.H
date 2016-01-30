/* 
 * Copyright (c) 2006-2011, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez SA BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
#include "raii.H"
#include <cmath>

using namespace raii;
using namespace raii::sql;
#include "Cairo.H"

namespace loudspeaker {

class Loudspeaker : public Object {

    public:

    String name;
    double qts;
    double vas;
    double fs;
    double z;
    double dB;
    double rms;
    double diameter;

    Loudspeaker(String _name="SCANSPEAK 26W/8534G00",
		double _qts=.4,double _vas=150,double _fs=23,
		double _z=8,double _dB=89,double _rms=80,
		double _diameter=27.2) :
        name(_name), qts(_qts), vas(_vas), fs(_fs), z(_z), dB(_dB), rms(_rms), diameter(_diameter) {}

    Loudspeaker(Connection& conn, String _name) :
        name("Lambda speaker"), qts(.5), vas(10), fs(30), z(8), dB(90), rms(100), diameter(21) {
            load(conn,_name);
}

    bool load(Connection& conn, String _name) {
        ResultSet rs=conn.query(String("SELECT * FROM loudspeaker WHERE name = '")+conn.sqlize(_name)+"'");
        if ( ! rs.next() ) {
            return false;
        }
        else {
            name=rs["name"];
            qts=rs["qts"].tof();
            vas=rs["vas"].tof();
            fs=rs["fs"].tof();
            z=rs["z"].tof();
            dB=rs["dB"].tof();
            rms=rs["rms"].tof();
            diameter=rs["diameter"].tof();
            return true;
        }
    }

    void insert(Connection& conn) {
        conn.query(String("INSERT INTO loudspeaker VALUES ('")+conn.sqlize(name)+"',"+ftostring(qts)+","+ftostring(vas)
                        +","+ftostring(fs)+","+ftostring(z)+","+ftostring(dB)+","+ftostring(rms)+","+ftostring(diameter)+")");
    }

    void update(Connection& conn) {
        conn.query(String("UPDATE loudspeaker SET qts=")+ftostring(qts)+", vas="+ftostring(vas)
                        +", fs="+ftostring(fs)+", z="+ftostring(z)+", dB="+ftostring(dB)+", rms="+ftostring(rms)+", diameter="+ftostring(diameter)+" WHERE name='"+conn.sqlize(name)+"'");
    }

    bool exists(Connection& conn) {
        ResultSet rs=conn.query(String("SELECT * FROM loudspeaker WHERE name='")+conn.sqlize(name)+"'");
        return rs.next();
    }

    void save(Connection& conn) {
        if ( exists(conn) )
            update(conn);
        else
            insert(conn);
    }

};

class Box : public Object {

    public:

    String name;
    double configuration;
    double fb;
    double Vb;
    double Vr;
    double Qb;
    double diameter;
    double entraxe;
    double L;
    double H;
    double laminaire;
    int filter;
    double fc;

    Box(String _name="Lambda box", double _configuration=1, double _fb=1,double _Vb=40, double _Vr=0.L, double _Qb=7,double _diameter=10, double _entraxe=0, double _L=20,double _H=2,bool _laminaire=false, int _filter=0, double _fc=0) :
        name(_name), configuration(_configuration), fb(_fb), Vb(_Vb), Vr(_Vr), Qb(_Qb), diameter(_diameter), entraxe(_entraxe), L(_L), H(_H), laminaire(_laminaire), filter(_filter), fc(_fc) {}

    Box(Connection& conn, String _name) :
        name("Lambda box"), configuration(1), fb(40), Vb(25), Vr(0.L), Qb(7), diameter(10), entraxe(0), L(20), H(2), laminaire(false), filter(0), fc(0) {
            load(conn,_name);
}

    bool load(Connection& conn, String _name) {
        ResultSet rs=conn.query(String("SELECT * FROM box WHERE name = '")+conn.sqlize(_name)+"'");
        if ( ! rs.next() ) {
            return false;
        }
        else {
            name=rs["name"];
            configuration=rs["configuration"].tof();
            fb=rs["fb"].tof();
            Vb=rs["Vb"].tof();
            Vr=rs["Vr"].tof();
            Qb=rs["Qb"].tof();
            diameter=rs["diameter"].tof();
            entraxe=rs["entraxe"].tof();
            L=rs["L"].tof();
            H=rs["H"].tof();
            laminaire=rs["laminaire"].toi();
            filter=rs["filter"].toi();
            fc=rs["fc"].tof();
            return true;
        }
    }

    void insert(Connection& conn) {
        conn.query(String("INSERT INTO box VALUES ('")+conn.sqlize(name)+"',"+ftostring(configuration)+","+ftostring(fb)+","+ftostring(Vb)+", "+ftostring(Vr)
                        +","+ftostring(Qb)+","+ftostring(diameter)+","+ftostring(entraxe)+","+ftostring(L)+","+ftostring(H)+","+itostring(laminaire)+","+itostring(filter)+","+ftostring(fc)+")");
    }

    void update(Connection& conn) {
        conn.query(String("UPDATE box SET configuration=")+ftostring(configuration)+", fb="+ftostring(fb)+", Vb="+ftostring(Vb)
			+", Vr="+ftostring(Vr)
                        +", Qb="+ftostring(Qb)+", diameter="+ftostring(diameter)+", entraxe="+ftostring(entraxe)+", L="+ftostring(L)+", H="+ftostring(H)+", laminaire="+itostring(laminaire)+", filter="+itostring(filter)+", fc="+ftostring(fc)+" WHERE name='"+conn.sqlize(name)+"'");
    }

    bool exists(Connection& conn) {
        ResultSet rs=conn.query(String("SELECT * FROM box WHERE name='")+conn.sqlize(name)+"'");
        return rs.next();
    }

    void save(Connection& conn) {
        if ( exists(conn) )
            update(conn);
        else
            insert(conn);
    }

};

class Curve : public Object {
    public:
    int id;
    String loudspeaker;
    String box;
    Box idealbox;
    double r;
    double g;
    double b;

    Curve(int i, double _r=0, double _g=0, double _b=0,String s1="", String s2="") : id(i), loudspeaker(s1), box(s2), idealbox(), r(_r), g(_g), b(_b) {}

   void calculateIdealBox(ptr<Loudspeaker> ls) {
	idealbox.Vb=20*pow(ls->qts,3.3L)*idealbox.configuration*ls->vas;
        idealbox.fb=pow(idealbox.configuration*ls->vas/idealbox.Vb,0.31L)*ls->fs;
   }

};

}
