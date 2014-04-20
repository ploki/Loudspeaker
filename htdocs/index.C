
#include "all.H"
#include <cmath>

namespace loudspeaker {

class SERVLET(index) : public HttpServlet {


        void service( HttpServletRequest& request, HttpServletResponse& response) {

                ptr<HttpSession> session = request.getSession();
                ptr<Loudspeaker> ls = session->getAttribute("loudspeaker");
                ptr<Box> box = session->getAttribute("box");
                ptr<List<Curve> > curves = session->getAttribute("curves");

                if ( ! ls ) {
                    ls = new Loudspeaker();
                    session->setAttribute("loudspeaker",ls);
                }

                if ( ! box ) {
                    box = new Box();
                    session->setAttribute("box",box);
                }

                if ( ! curves ) {
                    curves = new List<Curve>();
                    session->setAttribute("curves",curves);
                }

                String action=request.getParameter("action");

                String normalize=request.getParameter("normalize");
		String maxspl=request.getParameter("maxspl");
		String sum=request.getParameter("sum");


                if ( action.empty() ) {

                    if ( !request.getParameter("name").empty() ) {
                        box->name=request.getParameter("name");
                        box->fb=request.getParameter("fb").tof();
                        box->Vb=request.getParameter("Vb").tof();
                        box->Vr=request.getParameter("Vr").tof();
                        box->Qb=request.getParameter("Qb").tof();
                        box->diameter=request.getParameter("diameter").tof();
                        box->entraxe=request.getParameter("entraxe").tof();
                        box->L=request.getParameter("L").tof();
                        box->H=request.getParameter("H").tof();
                        box->laminaire=request.getParameter("laminaire").toi();
                        box->configuration=request.getParameter("configuration").tof();
			box->filter=request.getParameter("filter").toi();
			box->fc=request.getParameter("fc").tof();

                        if ( normalize.empty() )
                            session->setAttribute("normalize",NULL);
                        else
                            session->setAttribute("normalize",new String("plop"));
                        if ( maxspl.empty() )
                            session->setAttribute("maxspl",NULL);
                        else
                            session->setAttribute("maxspl",new String("plop"));
                        if ( sum.empty() )
                            session->setAttribute("sum",NULL);
                        else
                            session->setAttribute("sum",new String("plop"));
                    }


                        if ( !request.getParameter("load_loudspeaker").empty() ) {
                            Connection conn;
                            ls->load(conn,request.getParameter("load"));
                            response.sendRedirect(request.getContextPath()+"/index.C");
                        }
                        else if ( request.getParameter("loudspeaker") == "Edit" ) {
                                Connection conn;
                                ls->load(conn,request.getParameter("load"));
                                response.sendRedirect(request.getContextPath()+"/index.C?action=speaker");
                        }
                        else if ( request.getParameter("loudspeaker") == "List" ) {
                                response.sendRedirect(request.getContextPath()+"/index.C?action=list");
                        }
                        else if ( !request.getParameter("load_box").empty() ) {
                            Connection conn;
                            box->load(conn,request.getParameter("boxtoload"));
                            response.sendRedirect(request.getContextPath()+"/index.C");
                        }
                        else if ( !request.getParameter("save_box").empty() ) {

				Logger log("loudspeaker:save");
				log(request.getRemoteAddr()+" saved box "+box->name);
                            Connection conn;
                            box->save(conn);
                            response.sendRedirect(request.getContextPath()+"/index.C");
                        }
                        else if ( !request.getParameter("calc_fb_4").empty() ) {
                                box->fb = 0.383*ls->fs/ls->qts;
                            response.sendRedirect(request.getContextPath()+"/index.C");
                        }
                        else if ( !request.getParameter("calc_fb_6").empty() ) {
                                box->fb = 0.276*ls->fs/ls->qts;
                            response.sendRedirect(request.getContextPath()+"/index.C");
                        }
                        else if ( !request.getParameter("force_al").empty() ) {
			  String force_al = request.getParameter("force_al");
			  double al;
			  if ( force_al == "*" )
			    al = request.getParameter("al").tof();
			  else
			    al = request.getParameter("force_al").tof();
			  box->Vb=al*box->configuration*ls->vas*ls->qts*ls->qts;
			  response.sendRedirect(request.getContextPath()+"/index.C");
                        }
			else if ( !request.getParameter("ideal").empty() ) {
			// from http://www.diysubwoofers.org/prt/ported1.htm
				box->Vr=0.L;
				box->Vb=20*pow(ls->qts,3.3L)*box->configuration*ls->vas;
				box->fb=pow(box->configuration*ls->vas/box->Vb,0.31L)*ls->fs;
				response.sendRedirect(request.getContextPath()+"/index.C");
			}
			else if ( !request.getParameter("ideal_sealed").empty() ) {
			// from http://www.diysubwoofers.org/sld/sealed1.htm
				box->Vr=0.L;
				box->Vb=box->configuration*ls->vas/(pow((sqrt(2)/2)/ls->qts,2)-1);
				//box->fb=0.L;
				box->fb=1.L;
				response.sendRedirect(request.getContextPath()+"/index.C");
			}
			else if ( !request.getParameter("ideal4th").empty() ) {
			// from http://www.diysubwoofers.org/bnd/4thord1.htm
				String ideal4th = request.getParameter("ideal4th");
				double Pa = request.getParameter("Pa").tof();

				double S=0;
				double b=0;
				if ( ideal4th == "1.25 dB" ) {
					S=0.5;b=1.2712L;
				}
				else if ( ideal4th == "0.35 dB" ) {
					S=0.6;b=0.9560L;
				}
				else {
					//0dB
					S=0.7;b=0.7206L;
				}
				double Qbp = 1/(pow(10.L,(-Pa/40.L))*2*S);
				double Fl = (sqrt(-b+(b*b+4*Qbp*Qbp))/2.L)*(ls->fs/ls->qts);
				double Fh = Fl+(b*ls->fs/ls->qts);
				box->fb=Qbp*ls->fs/ls->qts;
				box->Vb=pow(2*S*ls->qts,2.L)*box->configuration*ls->vas;
				box->Vr=box->configuration*ls->vas/(pow(Qbp/ls->qts,2)-1);
				response.sendRedirect(request.getContextPath()+"/index.C");
			}
                        RequestDispatcher rd=request.getRequestDispatcher("/view/box.csp");
                        rd.forward(request,response);
                }


                else if ( action == "speaker" ) {


                    if ( !request.getParameter("name").empty() ) {
                        ls->name=request.getParameter("name");
                        ls->qts=request.getParameter("qts").tof();
                        ls->vas=request.getParameter("vas").tof();
                        ls->fs=request.getParameter("fs").tof();
                        ls->z=request.getParameter("z").tof();
                        ls->dB=request.getParameter("dB").tof();
                        ls->rms=request.getParameter("rms").tof();
                        ls->diameter=request.getParameter("diameter").tof();
                    }

                        if ( !request.getParameter("save_loudspeaker").empty() ) {
				Logger log("loudspeaker:save");
				log(request.getRemoteAddr()+" saved loudspeaker "+ls->name);
                            Connection conn;
                            ls->save(conn);
                            response.sendRedirect(request.getContextPath()+"/index.C");
                        }
                        else if ( !request.getParameter("load_loudspeaker").empty() ) {
                            Connection conn;
                            ls->load(conn,request.getParameter("load"));
                            response.sendRedirect(request.getContextPath()+"/index.C?action=speaker");
                        }
                        else if ( !request.getParameter("useit").empty() ) {
                                response.sendRedirect(request.getContextPath()+"/index.C");
                        }
                        RequestDispatcher rd=request.getRequestDispatcher("/view/speaker.csp");
                        rd.forward(request,response);

                }
                else if ( action == "list" ) {
                        request.getRequestDispatcher("/view/list.csp").forward(request,response);                        
                }
                else if ( action == "addcurve" ) {
                    curves->push_back(Curve(request.getParameter("id").toi(),
                                            request.getParameter("r").tof(),
                                            request.getParameter("g").tof(),
                                            request.getParameter("b").tof(),
                                            request.getParameter("loudspeaker"),
                                            request.getParameter("box")));
                    response.sendRedirect(request.getContextPath()+"/index.C");
                }
                else if ( action == "removecurve" ) {
                    int id = request.getParameter("id").toi();
                        for ( List<Curve>::iterator it=curves->begin() ;
                            it != curves->end() ;
                            ++it ) {
                            if ( it->id == id ) {
                                curves->erase(it);
                                break;
                            }
                        }
                     response.sendRedirect(request.getContextPath()+"/index.C");
                }
                else {
                        throw ServletException("Unknown action");
                }
        }
};

exportServlet(index);

}
