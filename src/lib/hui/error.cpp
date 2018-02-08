/*
 * hui_error.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "hui.h"
#include "internal.h"



#ifdef _X_USE_NET_
	#include "../net/net.h"
#endif

#include <signal.h>

namespace hui
{

extern Callback _idle_function_, _error_function_;

void _HuiSignalHandler(int)
{
	_error_function_();
}

// apply a function to be executed when a critical error occures
void SetErrorFunction(const Callback &function)
{
	_error_function_ = function;
	if (function){
		signal(SIGSEGV, &_HuiSignalHandler);
		/*signal(SIGINT, &_HuiSignalHandler);
		signal(SIGILL, &_HuiSignalHandler);
		signal(SIGTERM, &_HuiSignalHandler);
		signal(SIGABRT, &_HuiSignalHandler);*/
		/*signal(SIGFPE, &_HuiSignalHandler);
		signal(SIGBREAK, &_HuiSignalHandler);
		signal(SIGABRT_COMPAT, &_HuiSignalHandler);*/
	}
}

static Callback _eh_cleanup_function_;


#ifdef _X_USE_NET_

class ReportDialog : public Dialog
{
public:
	ReportDialog(Window *parent) :
		Dialog(_("Bug Report"), 450, 400, parent, false)
	{
		addGrid("", 0, 0, "root");
		setTarget("root");

		addGroup(_("Name:"), 0, 0, "grp_name");
		addGroup(_("Comment/what happened:"), 0, 1, "grp_comment");
		addLabel("!wrap//" + _("Your comments and the contents of the file message.txt will be sent."), 0, 2, "t_explanation");
		addGrid("!buttonbar", 0, 3, "buttonbar");

		setTarget("grp_name");
		addEdit(_("(anonymous)"), 0, 0, "sender");

		setTarget("grp_comment");
		addMultilineEdit("!expandy,expandx//" + _("Just happened somehow..."), 0, 0, "comment");

		setTarget("buttonbar");
		addButton(_("Cancel"),0, 0,"cancel");
		setImage("cancel", "hui:cancel");
		addDefButton(_("Ok"), 1, 0 ,"ok");
		setImage("ok", "hui:ok");

		event("ok", std::bind(&ReportDialog::onOk, this));
		event("cancel", std::bind(&ReportDialog::destroy, this));
		event("hui:close", std::bind(&ReportDialog::destroy, this));
	}

	void onOk()
	{
		string sender = getString("sender");
		string comment = getString("comment");
		string return_msg;
		if (NetSendBugReport(sender, Application::getProperty("name"), Application::getProperty("version"), comment, return_msg))
			InfoBox(NULL, "ok", return_msg);
		else
			ErrorBox(NULL, "error", return_msg);
		destroy();
	}
};

void SendBugReport(Window *parent)
{
	ReportDialog *dlg = new ReportDialog(parent);
	dlg->run();
	delete(dlg);
}

#endif

class ErrorDialog : public Dialog
{
public:
	ErrorDialog() :
		Dialog(_("Error"), 600, 500, NULL, false)
	{
		addGrid("", 0, 0, "root");
		setTarget("root");
		addLabel(Application::getProperty("name") + " " + Application::getProperty("version") + _(" has crashed.\t\tThe last lines of the file message.txt:"), 0, 0, "error_header");
		addListView(_("Messages"), 0, 1, "message_list");
		addGrid("!buttonbar", 0, 2, "buttonbar");
		setTarget("buttonbar");
		addButton(_("open message.txt"), 0, 0, "show_log");
		addButton(_("Send bug report to Michi"), 1, 0, "send_report");
		addButton(_("Ok"), 2, 0, "ok");
		setImage("ok", "hui:ok");

	#ifdef _X_USE_NET_
		event("send_report", std::bind(&ErrorDialog::onSendBugReport, this));
	#else
		enable("send_report", false);
		setTooltip("send_report", _("Program was compiled without network support..."));
	#endif
		for (int i=1023;i>=0;i--){
			string temp = msg_get_str(i);
			if (temp.num > 0)
				addString("message_list", temp);
		}
		event("show_log", std::bind(&ErrorDialog::onShowLog, this));
		//event("cancel", std::bind(&ErrorDialog::onClose, this));
		event("hui:win_close", std::bind(&ErrorDialog::onClose, this));
		event("ok", std::bind(&ErrorDialog::onClose, this));
	}

	void onShowLog()
	{
		OpenDocument("message.txt");
	}

	void onSendBugReport()
	{
		SendBugReport(this);
	}

	void onClose()
	{
		msg_write("real close");
		exit(0);
	}
};

void hui_default_error_handler()
{
	_idle_function_ = NULL;

	msg_reset_shift();
	msg_write("");
	msg_write("================================================================================");
	msg_write(_("program has crashed, error handler has been called... maybe SegFault... m(-_-)m"));
	//msg_write("---");
	msg_write("      trace:");
	msg_write(msg_get_trace());

	if (_eh_cleanup_function_){
		msg_write(_("i'm now going to clean up..."));
		_eh_cleanup_function_();
		msg_write(_("...done"));
	}

	foreachb(Window *w, _all_windows_)
		delete(w);
	msg_write(_("                  Close dialog box to exit program."));

	//HuiMultiline=true;
	//ComboBoxSeparator = "$";

	//HuiErrorBox(NULL,"Fehler","Fehler");

	// dialog
	ErrorDialog *dlg = new ErrorDialog;
	dlg->run();

	//HuiEnd();
	exit(0);
}

void SetDefaultErrorHandler(const Callback &error_cleanup_function)
{
	_eh_cleanup_function_ = error_cleanup_function;
	SetErrorFunction(&hui_default_error_handler);
}

void RaiseError(const string &message)
{
	msg_error(message + " (HuiRaiseError)");
	/*int *p_i=NULL;
	*p_i=4;*/
	hui_default_error_handler();
}


};

