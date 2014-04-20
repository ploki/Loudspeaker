#include <cmath>
#include "all.H"

//#define w (668)
#define w (501)
#define h (501)

#define mindb (40)
#define maxdb (120)
#define normdb (90)
#define plop (80)

#define LOG_START (1.L)
//#define LOG_END   (1.602L)
#define LOG_END   (2.L)

namespace loudspeaker {

class SERVLET(curve) : public HttpServlet {

        void curve(bool sum, double * total, bool normalize, bool maxspl, Cairo::Document& img, Cairo::Color color, ptr<Loudspeaker> ls, ptr<Box> box) {
                img.setColor(color);
                double H=box->fb/ls->fs;
                double A=box->configuration*ls->vas/box->Vb;
                double lv=0;
		double corr_db=0;
		double z=ls->z;
		if (box->configuration != 1 ) {
			corr_db=+3;
			z/=2.L;
		}
		if ( maxspl )
			corr_db+=10.L*log(ls->rms)/log(10.L);
                int li=0;

                img.setLineWidth(1.);
                if ( normalize ) {
                    img.line(0,h-((normdb-mindb)*h/plop),w,h-((normdb-mindb)*h/plop));
                    img.line(0,h-((normdb-mindb-3)*h/plop),w,h-((normdb-mindb-3)*h/plop));
                }
                else {
                    img.line(0,h-((corr_db+ls->dB-mindb)*h/plop),w,h-((corr_db+ls->dB-mindb)*h/plop));
                    img.line(0,h-((corr_db+ls->dB-mindb-3)*h/plop),w,h-((corr_db+ls->dB-mindb-3)*h/plop));
                }

                img.setLineWidth(2);

		//for sealed box
		double rvas = box->configuration*ls->vas;
		double Qtc = sqrt((rvas*ls->qts*ls->qts+box->Vb*ls->qts*ls->qts)/box->Vb);
		double fb = ls->fs * Qtc / ls->qts;
		//~
		bool odd=false;

                for ( int i = 0 ; i < w ; i+=1 ) {
			odd=!odd;
                        double f=pow(10,LOG_END*double(i)/w+LOG_START);

                        double v;
			if ( box->fb != 0 ) {
				if ( box->Vr != 0 ) {
					//4th order passband
			double A = pow(1/box->fb,2.L)*pow(f,4.L);
			double B = ((1/box->Qb+(ls->fs/box->fb)/ls->qts)/box->fb)*pow(f,3.L);
			double C = (((1+box->configuration*ls->vas/box->Vr+box->configuration*ls->vas/box->Vb)*ls->fs/box->fb+(1/ls->qts)/box->Qb)*ls->fs/box->fb+1)*pow(f,2.L);
			double D = ((1/ls->qts+(ls->fs/box->fb)/box->Qb*(box->configuration*ls->vas/box->Vr+1))*ls->fs)*f;
			double E = (box->configuration*ls->vas/box->Vr+1)*pow(ls->fs,2.L);
			double G = A-C+E;
			double H = -B+D;
			v=(normalize?0:corr_db) + 
		20 * log(pow(f,2.L)/sqrt(pow(G,2.L)+pow(H,2.L)));
				}
				else {
				//vented box
			v=(normalize?0:corr_db) + 20*log (pow(f/ls->fs,4.L)/pow(H,2)/(sqrt(pow(1-pow(f/ls->fs,2.L)*(1+(1+A)/pow(H,2)+1/(H*box->Qb*ls->qts))
                        +(pow(f/ls->fs,4))/pow(H,2),2)+pow((f/ls->fs)*(1/H/box->Qb+1/ls->qts)-(pow(f/ls->fs,3))*(1/ls->qts/pow(H,2)+1/H/box->Qb),2))))/log(10);
				}
			}
			else {
				//sealed box
			double fr = pow( f/fb,2.L ) ; 
//était à 10 mais 5 semble plus crédible
#define FACTOR (5)
			v= (normalize?0:corr_db) + FACTOR*log(pow(fr,2.L)/(pow(fr-1,2.L)+fr/pow(Qtc,2.L)));
			}
                        //v=-(v-20)*h/100;
			if ( box->filter != 0 ) {
				// (2) * car on parle de P pas de V
				// ou (1) * car la tension tombe à 70% et la puissance /2 donc -3dB 
				v -= 1 * 10 * log ( 1.L + pow( f / box->fc, box->filter ) )/log(10);
				//se démontre avec les 2 lignes :
				//double t = pow(sqrt(pow(10.L,v/10.L))/(1.L+pow( f / box->fc , box->filter )),2.L);
				//v= 10.L*log(t)/log(10);
			}

                        if (normalize) {
			    if ( sum )
				total[i]+=pow(10.L,(v+normdb-mindb)/10.L);
                            v= h- ( (v+normdb-mindb) * h/plop );
			}
			else {
			    if ( sum )
				total[i]+=pow(10.L,(v+ls->dB-mindb)/10.L);
                            v= h- ( (v+ls->dB-mindb) * h/plop );
			}

                        if ( lv == 0 )
                                lv=v;
			if ( z >= 6.L || odd )
	                        img.line(li,lv,i,v);
                        lv=v;li=i;
                }
        }

        void service( HttpServletRequest& request, HttpServletResponse& response) {

                ptr<HttpSession> session = request.getSession();
                ptr<Loudspeaker> ls = session->getAttribute("loudspeaker");
                ptr<Box> box = session->getAttribute("box");
                ptr<List<Curve> > curves = session->getAttribute("curves");
                bool normalize = session->getAttribute("normalize")!=NULL;
		bool maxspl = session->getAttribute("maxspl")!=NULL;
		bool sum = session->getAttribute("sum")!=NULL;

                if ( ! ls ) {
                    ls = new Loudspeaker();
                    session->setAttribute("loudspeaker",ls);
                }

                if ( ! box ) {
                    box = new Box();
                    session->setAttribute("box",box);
                }

//                response.setContentType("image/png");
//                Cairo::Document img("",w,h,Cairo::ARGB);
		bool svg=getInitParameter("ImageType")=="SVG"  &&  ( ! request.getHeader("User-Agent").matches("MSIE") );
                response.setContentType(svg?"image/svg+xml":"image/png");
		float r=getInitParameter("R").tof();
		float g=getInitParameter("G").tof();
		float b=getInitParameter("B").tof();
                Cairo::Document img("",w,h,svg?Cairo::SVG:Cairo::ARGB);
                //cadre
                img.beginPage();
                img.setColor(Cairo::Color::RGB(r,g,b));
                img.line(0,0,w-1,0);
                img.line(0,h-1,w-1,h-1);
                img.line(0,0,0,h-1);
                img.line(w-1,0,w-1,h-1);
                int k=0;
                for ( int i = 0 ; i < 10 ; ++i )
                        for ( int j = 0 ; j < int(LOG_END+1) ; j++ ) {
                        k++;
                        img.setLineWidth(1./(i+1.));

                        double f= i*pow(10,j);
                        int x=int( (log(f)/log(10.L)-LOG_START)*w/LOG_END );
                        img.line(x,0,x,h);
                        img.moveTo(x-10,k%2?h-25:0);
                        img.addLabel(itostring(i*pow(10,j)));
                }

                for ( int y = mindb ; y <maxdb ; y+=5 ) {
                        if ( y%10 == 0 ) {
                            img.setLineWidth(1./5.);
                            //img.moveTo(0,mindb+(plop*(y-h)/(-h)));
                            img.line(0,h-((y-mindb)*h/plop),w,h-((y-mindb)*h/plop));
                            img.moveTo(4,h-((y-mindb)*h/plop)-20);
                            img.addLabel(itostring(y-(normalize?normdb:0))+"dB");
                        }
                        else {
                            img.setLineWidth(1./10.);
                            //img.moveTo(0,mindb+(plop*(y-h)/(-h)));
                            img.line(0,h-((y-mindb)*h/plop),w,h-((y-mindb)*h/plop));
                        }
                }


                //fin cadre

                ptr<Loudspeaker> other_ls = new Loudspeaker();
                ptr<Box> other_box= new Box();
		double total[w];
		for (int i = 0 ; i < w ; ++i ) total[i]=0.L;
		if ( curves )
                for ( List<Curve>::iterator it=curves->begin() ;
                            it != curves->end() ;
                            ++it ) {
                    Connection conn;
                    other_ls->load(conn,it->loudspeaker);
			if ( it->box != "¤" )
	                    other_box->load(conn,it->box);
			else {
                           it->calculateIdealBox(other_ls);
			   *other_box=it->idealbox;
			}
                    curve(sum,&(total[0]),normalize,maxspl,img,Cairo::Color::RGB(
                        (sum?2.L/3.L:0.L)+it->r/(sum?3.L:1.L),
                        (sum?2.L/3.L:0.L)+it->g/(sum?3.L:1.L),
                        (sum?2.L/3.L:0.L)+it->b/(sum?3.L:1.L)
                        ),other_ls,other_box);
                }
                
		curve(sum,&(total[0]),normalize,maxspl,img,Cairo::Color::RGB(
                      sum?2.L/3.L:0.L,
                      sum?2.L/3.L:0.L,
                      sum?2.L/3.L:0.L
                    ),ls,box);

		if ( sum ) {
                	img.setColor(Cairo::Color::RGB(0,0,0));
                	img.setLineWidth(2.);
			int li=0,lv=0;
			for (int i = 0 ; i < w ; ++i ) {
				total[i]=10*log(total[i])/log(10.L);
                            	int v= h- ( (total[i]) * h/plop );
                        	if ( lv == 0 )
                                	lv=v;
	                        img.line(li,lv,i,v);
                        	lv=v;li=i;
			}
		}
		
              img.endPage();

        }
};

exportServlet(curve);

}
