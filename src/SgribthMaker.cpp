#include "lib/base/base.h"
#include "lib/hui/hui.h"
#include "lib/nix/nix.h"
#include "lib/file/file.h"
#include "lib/x/x.h"
#include "lib/script/script.h"
#include <gtk/gtk.h>
#include "lib/algebra/algebra.h"


GtkTextBuffer *tb;
GtkWidget *tv;

string Filename = "";


string AppTitle = "SgribthMaker";
string AppVersion = "0.3.19.2";

#define ALLOW_LOGGING			true
//#define ALLOW_LOGGING			false

#define MAX_HIGHLIGHTING_SIZE	100000

CHuiWindow *MainWin;
CHuiWindow *CommandDialog = NULL, *MessageDialog;
string LastCommand;
int timer, CompileTimer;

#define COLORMODE		1



//------------------------------------------------------------------------------
// Undo/Redo

bool Changed = false;
bool HistoryEnabled = false;


#define UndoInsert		0
#define UndoRemove		1

int UndoStackPos = 0;
int UndoStackSavedPos = -1;
struct s_undo{
	char type;
	int pos, length;
	char *text;
};
Array<s_undo> undo;

int NeedsUpdateStart = 0, NeedsUpdateEnd = 0;

//------------------------------------------------------------------------------
// Highlighting

enum{
	CharSpace,
	CharLetter,
	CharNumber,
	CharSign
};

enum{
	InWordSpecial,
	InWordCompilerFunction,
	InWordGameVariable,
	InWordType,
	InWord,
	InLineComment,
	InCommentLevel1,
	InCommentLevel2,
	InMacro,
	InSpace,
	InString,
	InOperator,
	InNumber,
	InSpecialCallback,
	NumTagTypes
};

/*enum{
	HighLightNone,
	HighLightKaba,
	HighLightLatex
};*/

struct TagType
{
	color fg, bg;
	bool set_bg;
	bool bold, italic;
	GtkTextTag *tag;
};

TagType tag[NumTagTypes];


struct sScriptFunction
{
	string name;
	int line;
};
Array<sScriptFunction> ScriptFunction;



inline int char_type(char c)
{
	if ((c >= '0') && (c <= '9'))
		return CharNumber;
	if ((c == ' ') || (c == '\n') || (c == '\t'))
		return CharSpace;
	if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_'))
		return CharLetter;
	return CharSign;
}

inline void MarkWord(int line, int start, int end, int type, char *p0, char *p)
{
	if (start == end)
		return;
	msg_db_r("MarkWord", 1);
	if (type == InWord)
		if ((start == 0) || (p0[-1] != '.'))
		if ((long)p - (long)p0 < 64){
			string temp;
			for (int i=0;i<(long)p - (long)p0;i++)
				temp.add(p0[i]);
			//msg_write(temp);
			if (temp[0] == '#'){
				type = InMacro;
			}else if ((temp == "enum") ||
			         //(temp == "struct") ||
			         (temp == "class") ||
			         (temp == "if") ||
			         (temp == "else") ||
			         (temp == "while") ||
			         (temp == "for") ||
			         (temp == "forall") ||
			         (temp == "in") ||
			         (temp == "return") ||
			         (temp == "break") ||
			         (temp == "continue") ||
			         (temp == "and") ||
			         (temp == "or") ||
			         (temp == "extern") ||
			         (temp == "const") ||
			         (temp == "this") ||
			         (temp == "self") ||
			         (temp == "asm")){
				type = InWordSpecial;
			}else{
				bool found = false;
				for (int i=0;i<Script::PreTypes.num;i++)
					if (temp == Script::PreTypes[i]->name){
						type = InWordType;
						found = true;
						break;
					}
				if (!found)
				for (int i=0;i<Script::PreExternalVars.num;i++)
					if (temp == Script::PreExternalVars[i].name){
						type = InWordGameVariable;
						found = true;
						break;
					}
				if (!found)
				for (int i=0;i<Script::PreConstants.num;i++)
					if (temp == Script::PreConstants[i].name){
						type = InWordGameVariable;
						found = true;
						break;
					}
				if (!found)
				for (int i=0;i<Script::PreCommands.num;i++)
					if (temp == Script::PreCommands[i].name){
						type = InWordCompilerFunction;
						found = true;
						break;
					}
			}
		}
	GtkTextIter _start, _end;
	gtk_text_buffer_get_iter_at_line_offset(tb, &_start, line, start);
	gtk_text_buffer_get_iter_at_line_offset(tb, &_end, line, end);
	gtk_text_buffer_apply_tag (tb, tag[type].tag, &_start, &_end);
	msg_db_l(1);
}

#define next_char()	p=g_utf8_next_char(p);pos++
#define set_mark()	p0=p;pos0=pos

bool allow_highlighting(const string &name)
{
	string extension = name.extension();
	if (extension == "kaba")
		return true;
	if ((extension == "cpp") || (extension == "h"))
		return true;
	if (name.basename() == "config.txt")
		return true;
	return false;
}

void CreateTextColors(int first_line = -1, int last_line = -1)
{
	if (gtk_text_buffer_get_char_count(tb) > MAX_HIGHLIGHTING_SIZE)
		return;
	if (!allow_highlighting(Filename))
		return;
	msg_db_r("CreateTextColors", 1);
	GtkTextIter start, end;

	int comment_level = 0;
	int num_lines = gtk_text_buffer_get_line_count(tb);
	if (first_line < 0)
		first_line = 0;
	if (last_line < 0)
		last_line = num_lines - 1;


	for (int l=first_line;l<=last_line;l++){
		gtk_text_buffer_get_iter_at_line_index(tb, &start, l, 0);
		gtk_text_buffer_get_iter_at_line_index(tb, &end, l, 0);
		while (!gtk_text_iter_ends_line(&end))
			if (!gtk_text_iter_forward_char(&end))
				break;


		gtk_text_buffer_remove_all_tags(tb,&start, &end);

		//msg_write("a");
		char *s = gtk_text_buffer_get_text(tb, &start, &end, false);
		char *p = s;
		char *p0 = s;
		int last_type = CharSpace;
		int in_type = (comment_level > 1) ? InCommentLevel2 : ((comment_level > 0) ? InCommentLevel1 : InSpace);
		int line_len = gtk_text_iter_get_offset(&end) - gtk_text_iter_get_offset(&start);
		int str_len = strlen(s);
		int pos0 = 0;
		int pos = 0;
		while(pos < line_len){
			int type = char_type(*p);
			// still in a string?
			if (in_type == InString){
				if (*p == '\"'){
					in_type = InOperator;
					next_char();
					MarkWord(l, pos0, pos, InString, p0, p);
					set_mark();
					continue;
				}
			// still in a multi-comment?
			}else if (comment_level > 0){
				if ((*p == '/') && (p[1] == '*')){
					MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					next_char();
					in_type = InCommentLevel2;
					comment_level ++;
				}else if ((*p == '*') && (p[1] == '/')){
					next_char();
					next_char();
					MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					comment_level --;
					in_type = (comment_level > 0) ? InCommentLevel1 : InOperator;
					last_type = type;
					continue;
				}
			}else{
				// string starting?
				if (*p == '\"'){
					MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					in_type = InString;
				}else if (last_type != type){
					if ((in_type == InNumber) && ((*p == '.') || (*p == 'x') || ((*p >= 'a') && (*p <= 'f')))){
						next_char();
						continue;
					}
					if ((in_type == InWord) && (type == CharNumber)){
						next_char();
						continue;
					}
					MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					if (type == CharSpace)
						in_type = InSpace;
					else if (type == CharLetter)
						in_type = InWord;
					else if (type == CharNumber)
						in_type = InNumber;
					else if (type == CharSign)
						in_type = InOperator;
					// # -> macro...
					if (*p == '#'){
						in_type = InWord;
						type = CharLetter;
					}
				}
				// line comment starting?
				if ((*p == '/') && (p[1] == '/')){
					MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					in_type = InLineComment;
					break;
				// multi-comment starting?
				}else if ((*p == '/') && (p[1] == '*')){
					MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					in_type = InCommentLevel1;
					comment_level ++;
					next_char();
				}
			}
			last_type = type;
			next_char();
		}
		if (line_len > 0)
			MarkWord(l, pos0, line_len, in_type, p0, &s[str_len]);
		g_free(s);
	}
	msg_db_l(1);
}

void FindScriptFunctions()
{
	msg_db_r("FindScriptFunctions", 1);
	ScriptFunction.clear();
	GtkTextIter start, end;
	int num_lines = gtk_text_buffer_get_line_count(tb);
	for (int l=0;l<num_lines;l++){
		int ll = 0;
		gtk_text_buffer_get_iter_at_line_index(tb, &start, l, 0);
		gtk_text_buffer_get_iter_at_line_index(tb, &end, l, 0);
		while (!gtk_text_iter_ends_line(&end))
			if (!gtk_text_iter_forward_char(&end))
				break;
			else
				ll ++;
		char *s = gtk_text_buffer_get_text(tb, &start, &end, false);
		if (char_type(s[0]) == CharLetter){
			for (int i=0;i<ll;i++)
				if (s[i]=='('){
					sScriptFunction f;
					f.name.resize(ll);
					memcpy(f.name.data, s, ll);
					f.line = l;
					ScriptFunction.add(f);
					break;
				}
		}
		g_free(s);
	}
	msg_db_l(1);
}

int status_count = 0;

void UpdateStatusBar()
{
	msg_db_r("UpdateStatusBar", 2);
	status_count --;
	if (status_count == 0)
		MainWin->EnableStatusbar(false);
	msg_db_l(2);
}

void SetMessage(const string &str)
{
	msg_db_r("SetMessage", 2);
	MainWin->SetStatusText(str);
	MainWin->EnableStatusbar(true);
	//HuiGetTime(timer);
	status_count ++;
	HuiRunLater(5000, &UpdateStatusBar);
	msg_db_l(2);
}

void SetWindowTitle()
{
	msg_db_r("SetWinTitle", 1);
	if (Changed){
		if (Filename.num > 0)
			MainWin->SetTitle("*" + Filename + " - " + AppTitle);
		else
			MainWin->SetTitle("*neues Dokument - " + AppTitle);
	}else{
		if (Filename.num)
			MainWin->SetTitle(Filename + " - " + AppTitle);
		else
			MainWin->SetTitle("neues Dokument - " + AppTitle);
	}
	msg_db_l(1);
}

void ResetHistory()
{
	msg_db_r("RestHistory", 1);
	for (int i=0;i<undo.num;i++)
		g_free(undo[i].text);
	undo.resize(0);
	UndoStackPos = 0;
	UndoStackSavedPos = 0;
	Changed = false;
	HistoryEnabled = true;
	MainWin->GetMenu()->EnableItem("undo", false);
	MainWin->GetMenu()->EnableItem("redo", false);
	msg_db_l(1);
}

void undo_insert_text(int pos, char *text, int length)
{
	msg_db_r("undo_insert_text", 1);
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_offset(tb, &start, pos);
	gtk_text_buffer_insert(tb, &start, text, length);
	gtk_text_buffer_place_cursor(tb, &start);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
	msg_db_l(1);
}

void undo_remove_text(int pos, char *text, int length)
{
	msg_db_r("undo_remove_text", 1);
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_offset(tb, &start, pos);
	gtk_text_buffer_get_iter_at_offset(tb, &end, pos);
	gtk_text_iter_forward_chars(&end, g_utf8_strlen(text, length));
	gtk_text_buffer_delete(tb, &start, &end);
	gtk_text_buffer_place_cursor(tb, &start);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
	msg_db_l(1);
}

void UpdateMenu()
{
	MainWin->GetMenu()->EnableItem("undo", UndoStackPos > 0);
	MainWin->GetMenu()->EnableItem("redo", true);
}

void Undo()
{
	if (UndoStackPos <= 0)
		return;
	msg_db_r("Undo", 1);
	HistoryEnabled = false;
	UndoStackPos --;
	s_undo *u = &undo[UndoStackPos];
	if (u->type == UndoRemove)
		undo_insert_text(u->pos, u->text, u->length);
	else
		undo_remove_text(u->pos, u->text, u->length);
	HistoryEnabled = true;
	Changed = (UndoStackPos != UndoStackSavedPos);
	SetWindowTitle();
	UpdateMenu();
	msg_db_l(1);
}

void Redo()
{
	if (UndoStackPos >= undo.num)
		return;
	msg_db_r("Redo", 1);
	HistoryEnabled = false;
	s_undo *u = &undo[UndoStackPos];
	if (u->type == UndoRemove)
		undo_remove_text(u->pos, u->text, u->length);
	else
		undo_insert_text(u->pos, u->text, u->length);
	UndoStackPos ++;
	HistoryEnabled = true;
	Changed = (UndoStackPos != UndoStackSavedPos);
	SetWindowTitle();
	UpdateMenu();
	msg_db_l(1);
}

void ChangeHistory(int type, char *text, int length, int pos)
{
	if (!HistoryEnabled)
		return;
	msg_db_r("ChangeHistory", 1);
	s_undo u;
	u.type = type;
	u.text = text;
	u.length = length;
	u.pos = pos;

	// remove illegal actions
	for (int i=UndoStackPos;i<undo.num;i++)
		g_free(undo[i].text);
	undo.resize(UndoStackPos);
	if (UndoStackPos < UndoStackSavedPos)
		UndoStackSavedPos = -1;

	// insert
	undo.add(u);
	UndoStackPos ++;
	Changed = true;
	UpdateMenu();
	msg_db_l(1);
}

bool Save();

bool AllowTermination()
{
	if (Changed){
		string answer = HuiQuestionBox(MainWin, _("dem&utige aber h&ofliche Frage"), _("Sie haben die Entropie erh&oht. Wollen Sie Ihr Werk speichern?"), true);
		if (answer == "cancel")
			return false;
		if (answer == "yes")
			return Save();
		return true;
	}
	return true;
}

void New()
{
	if (!AllowTermination())
		return;

	msg_db_r("New", 1);
	HistoryEnabled = false;
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(tb, &start, &end);
	gtk_text_buffer_delete(tb, &start, &end);
	gtk_text_buffer_set_modified(tb, false);

	Filename = "";
	ResetHistory();
	SetWindowTitle();
	msg_db_l(1);
}

string convert_to_utf8(string temp)
{
	msg_db_r("convert_to_utf8", 1);
	string utf8;// = new char[strlen(temp)];
	const char *t = temp.c_str();
	while(*t){
		gunichar a = g_utf8_get_char_validated(t, -1);
		if (a != -1)
			utf8.append_1_single(a);
		else
			utf8.append_1_single('?');
		t ++;
	}
	/*return utf8;
	gunichar *aa = g_utf8_to_ucs4(temp, -1, NULL, NULL, NULL);
	char *utf8 = g_ucs4_to_utf8(aa, -1, NULL, NULL, NULL);
	g_free(aa);*/
	msg_db_l(1);
	return utf8;
}

bool LoadFromFile(const string &filename)
{
	msg_db_r("LoadFromFile", 1);
	CFile *f = OpenFile(filename);
	if (!f){
		SetMessage(_("Datei l&asst sich nicht &offnen"));
		msg_db_l(1);
		return false;
	}
	
	HistoryEnabled = false;
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(tb, &start, &end);
	gtk_text_buffer_delete(tb, &start, &end);
	ResetHistory();

	HistoryEnabled = false;
	string temp = f->ReadComplete();
	FileClose(f);

	char **val_end;
	if (g_utf8_validate((char*)temp.data, temp.num, NULL)){
		gtk_text_buffer_set_text(tb, temp.c_str(), -1);
		gtk_text_buffer_set_modified(tb, false);
		temp.clear();
		ResetHistory();
	}else{
		SetMessage(_("Datei nicht UTF-8 kompatibel"));
		string temp_utf8 = convert_to_utf8(temp);
		temp.clear();
		gtk_text_buffer_set_text(tb, temp_utf8.c_str(), -1);
		gtk_text_buffer_set_modified(tb, true);
		temp_utf8.clear();
		ResetHistory();
		UndoStackSavedPos = -1;
		Changed = true;
	}

	Filename = filename;

	CreateTextColors();

	//GtkTextIter start;
	gtk_text_buffer_get_start_iter(tb, &start);
	gtk_text_buffer_place_cursor(tb, &start);
	SetWindowTitle();
	msg_db_l(1);
	return true;
}

bool WriteToFile(const string &filename)
{
	msg_db_r("WriteToFile", 1);
	CFile *f = CreateFile(filename);
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(tb, &start, &end);
	char *temp = gtk_text_buffer_get_text(tb, &start, &end, false);
	f->WriteBuffer(temp, strlen(temp));
	FileClose(f);
	g_free(temp);
	gtk_text_buffer_set_modified(tb, false);

	Filename = filename;
	UndoStackSavedPos = UndoStackPos;
	Changed = false;
	SetWindowTitle();
	SetMessage(_("gespeichert"));
	msg_db_l(1);
	return true;
}

bool Open()
{
	if (!AllowTermination())
		return false;

	if (HuiFileDialogOpen(MainWin, _("Datei &offnen"), Filename.dirname(), _("Alles (*.*)"), "*"))
		return LoadFromFile(HuiFilename);
	return false;
}

bool SaveAs()
{
	if (HuiFileDialogSave(MainWin, _("Datei speichern"), Filename.dirname(), _("Alles (*.*)"), "*"))
		return WriteToFile(HuiFilename);
	return false;
}

bool Save()
{
	if (Filename.num > 0)
		return WriteToFile(Filename);
	else
		return SaveAs();
}

void OnOpen()
{	Open();	}

void OnSave()
{	Save();	}

void OnSaveAs()
{	SaveAs();	}

bool Reload()
{
	if (!AllowTermination())
		return false;
	if (Filename.num > 0){
		bool r = LoadFromFile(Filename);
		if (r)
			SetMessage(_("neu geladen"));
		return r;
	}
	return true;
}

void OnReload()
{	Reload();	}

void CopyToClipboard()
{
	GtkClipboard *cb=gtk_clipboard_get_for_display(gdk_display_get_default(),GDK_SELECTION_CLIPBOARD);
	gtk_text_buffer_copy_clipboard(tb, cb);
	SetMessage(_("kopiert"));
}

void PasteFromClipboard()
{
	msg_db_r("PasteFromCB", 1);
	GtkClipboard *cb=gtk_clipboard_get_for_display(gdk_display_get_default(),GDK_SELECTION_CLIPBOARD);
	//gtk_text_buffer_paste_clipboard(tb, cb, NULL, true);
	char *text = gtk_clipboard_wait_for_text(cb);
	if (text){
		gtk_text_buffer_insert_at_cursor(tb, text, -1);
		gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
		g_free(text);
	}
	SetMessage(_("eingef&ugt"));
	msg_db_l(1);
}

void DeleteSelection()
{
	msg_db_r("DeleteSel", 1);
	gtk_text_buffer_delete_selection(tb, true, true);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
	msg_db_l(1);
}

void Cut()
{
	CopyToClipboard();
	DeleteSelection();
}

void JumpToStartOfLine(bool shift)
{
	msg_db_r("JumpToStartOfLine", 1);
	GtkTextMark *mi = gtk_text_buffer_get_insert(tb);
	GtkTextMark *msb = gtk_text_buffer_get_selection_bound(tb);
	GtkTextIter ii, isb, i0, i1;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, mi);
	gtk_text_buffer_get_iter_at_mark(tb, &isb, msb);

	int line = gtk_text_iter_get_line(&ii);
	gtk_text_buffer_get_iter_at_line_index(tb, &i0, line, 0);
	i1 = i0;
	while (!gtk_text_iter_ends_line(&i1)){
		int c = gtk_text_iter_get_char(&i1);
		if (!g_unichar_isspace(c))
			break;
		if (!gtk_text_iter_forward_char(&i1))
			break;
	}
	ii = (gtk_text_iter_equal(&i1, &ii)) ? i0 : i1;
	if (shift)
		gtk_text_buffer_select_range(tb, &ii, &isb);
	else
		gtk_text_buffer_place_cursor(tb, &ii);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
	msg_db_l(1);
}

void JumpToEndOfLine(bool shift)
{
	msg_db_r("JumpToEndOfLine", 1);
	GtkTextMark *mi = gtk_text_buffer_get_insert(tb);
	GtkTextMark *msb = gtk_text_buffer_get_selection_bound(tb);
	GtkTextIter ii, isb;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, mi);
	gtk_text_buffer_get_iter_at_mark(tb, &isb, msb);

	int line = gtk_text_iter_get_line(&ii);
	gtk_text_buffer_get_iter_at_line_index(tb, &ii, line, 0);
	while (!gtk_text_iter_ends_line(&ii))
		if (!gtk_text_iter_forward_char(&ii))
			break;
	if (shift)
		gtk_text_buffer_select_range(tb, &ii, &isb);
	else
		gtk_text_buffer_place_cursor(tb, &ii);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
	msg_db_l(1);
}

void MoveCursorTo(int line, int pos)
{
	msg_db_r("MoveCursorTo", 1);
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_line_index(tb, &iter, line, 0);
	while (!gtk_text_iter_ends_line(&iter))
		if (!gtk_text_iter_forward_char(&iter))
			break;
	if (gtk_text_iter_get_line_index(&iter) > pos)
		gtk_text_buffer_get_iter_at_line_index(tb, &iter, line, pos);
	gtk_text_buffer_place_cursor(tb, &iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
	msg_db_l(1);
}

void InsertReturn()
{
	msg_db_r("InsertReturn", 1);
	GtkTextMark *mi = gtk_text_buffer_get_insert(tb);
	GtkTextIter ii, i0, i1;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, mi);
	int line = gtk_text_iter_get_line(&ii);
	gtk_text_buffer_get_iter_at_line_index(tb, &i0, line, 0);
	i1 = i0;
	while ((!gtk_text_iter_ends_line(&i1)) && (!gtk_text_iter_equal(&i1, &ii))){
		int c = gtk_text_iter_get_char(&i1);
		if (!g_unichar_isspace(c))
			break;
		if (!gtk_text_iter_forward_char(&i1))
			break;
	}
	char *text = gtk_text_buffer_get_text(tb, &i0, &i1, false);
	gtk_text_buffer_insert_at_cursor(tb, "\n", -1);
	gtk_text_buffer_insert_at_cursor(tb, text, -1);
	g_free(text);
	msg_db_l(1);
}

string get_time_str(float t)
{
	if (t < 0.3f)
		return format("%.2fms", t * 1000.0f);
	else
		return format("%.2fs", t);
}

void Compile()
{
	if (Filename.extension() != "kaba"){
		SetMessage(_("nur *.kaba-Dataien k&onnen &ubersetzt werden!"));
		return;
	}

	if (!Save())
		return;

	msg_db_r("Compile",1);

	//HuiSetDirectory(SgribthDir);
	msg_set_verbose(true);

	HuiGetTime(CompileTimer);

	Script::CompileSilently = true;
	msg_write(Filename);
	Script::Script *compile_script = Script::Load(Filename, true, true);

	float dt = HuiGetTime(CompileTimer);

	//compile_script->Show();

	if (compile_script->Error){
		HuiErrorBox(MainWin, _("Fehler"), compile_script->ErrorMsgExt[0] + "\n" + compile_script->ErrorMsgExt[1]);
		MoveCursorTo(compile_script->ErrorLine, compile_script->ErrorColumn);
	}else
		SetMessage(format(_("Script ist fehler-frei &ubersetzbar!        (in %s)"), get_time_str(dt).c_str()));

	//RemoveScript(compile_script);
	Script::DeleteAllScripts(true, true);

	msg_db_m("set verbose...",1);
	msg_set_verbose(ALLOW_LOGGING);
	msg_db_l(1);
}

void OnMessageDialogClose()
{	delete(MessageDialog);	}

void CompileAndRun(bool verbose)
{
	if (Filename.extension() != "kaba"){
		SetMessage(_("nur *.kaba-Dataien k&onnen &ubersetzt werden!"));
		return;
	}

	if (!Save())
		return;

	msg_db_r("CompileAndRun",1);

	HuiSetDirectory(Filename);
	//if (verbose)
		msg_set_verbose(true);

	// compile
	HuiGetTime(CompileTimer);
	Script::CompileSilently = true;
	Script::Script *compile_script = Script::Load(Filename);
	float dt_compile = HuiGetTime(CompileTimer);

	if (compile_script->Error){
		HuiErrorBox(MainWin,_("Fehler"), compile_script->ErrorMsgExt[0] + "\n" + compile_script->ErrorMsgExt[1]);
		MoveCursorTo(compile_script->ErrorLine, compile_script->ErrorColumn);
	}else{

		void *x = NULL;
		//compile_script->SetVariable("this", &x);

		if (!verbose)
			msg_set_verbose(true);

		int msg_size0 = msg_get_buffer_size();


		float dt_execute = 0;
		if (compile_script->pre_script->FlagCompileOS)
			HuiErrorBox(MainWin, _("Fehler"), _("Script nicht ausf&uhrbar. (#os)"));
		else{
			HuiPushMainLevel();
			HuiGetTime(CompileTimer);
			if (!compile_script->pre_script->FlagNoExecution){
				compile_script->Execute();
				while(compile_script->WaitingMode != 0){
					//msg_write(string("kleiner Test", HuiAppDirectory));
					compile_script->Execute();
				}
			}
			dt_execute = HuiGetTime(CompileTimer);
			HuiPopMainLevel();
		}
		
		if (compile_script->Error)
			HuiErrorBox(MainWin,_("Fehler"), _("beim Ausf&uhren des Scriptes:\n") + compile_script->ErrorMsgExt[0]);
		else
			SetMessage(format(_("Compilieren: %s         Opcode: %db         Ausf&uhren: %s"), get_time_str(dt_compile).c_str(), compile_script->OpcodeSize, get_time_str(dt_execute).c_str()));
		//if (verbose)
		//	HuiInfoBox(MainWin,"Speicher",string("nicht freigegebener Speicher des Scriptes: ",i2s(script->MemoryUsed),"b"));}

		// messages?
		int msg_size = msg_get_buffer_size();
		if (msg_size > msg_size0){
			MainWin->HideControl("table_console", false);
			MainWin->SetString("log", msg_get_buffer(msg_size - msg_size0));
		}

	}
	

	//RemoveScript(compile_script);
	Script::DeleteAllScripts(true, true);

	msg_set_verbose(ALLOW_LOGGING);

	msg_db_l(1);
}

void OnCompileAndRunVerbose()
{	CompileAndRun(true);	}

void OnCompileAndRunSilent()
{	CompileAndRun(false);	}


void ShowCurLine()
{
	msg_db_r("ShowCurLine", 1);
	GtkTextIter ii;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, gtk_text_buffer_get_insert(tb));
	int line = gtk_text_iter_get_line(&ii);
	int off = gtk_text_iter_get_line_offset(&ii);
	SetMessage(format(_("Zeile  %d : %d"), line + 1, off + 1));
	msg_db_l(1);
}

void ExecuteCommand(string &cmd)
{
	msg_db_r("ExecCmd", 1);
	// find...
	GtkTextIter ii, isb;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, gtk_text_buffer_get_insert(tb));
	int off = gtk_text_iter_get_offset(&ii);
	//int nn = gtk_text_buffer_get_char_count(tb);
	
	string temp;
	temp.resize(cmd.num);
	for (int i=0;i<cmd.num;i++){
		temp[i] = gtk_text_iter_get_char(&ii);
		if (gtk_text_iter_forward_char(&ii))
			break;
	}

	bool found = false;
	while(true){
		if (temp == cmd){
			found = true;
			break;
		}
		if (!gtk_text_iter_forward_char(&ii))
			break;
		for (int i=0;i<cmd.num - 1;i++)
			temp[i] = temp[i + 1];
		temp[cmd.num - 1] = gtk_text_iter_get_char(&ii);
	}
	if (found){
		gtk_text_iter_forward_char(&ii);
		isb = ii;
		gtk_text_iter_backward_chars(&ii, cmd.num);
		gtk_text_buffer_select_range(tb, &ii, &isb);
		gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
	}else
		SetMessage(format(_("\"%s\" nicht gefunden"), cmd.c_str()));
	msg_db_l(1);
}

void OnCommandOk()
{
	LastCommand = CommandDialog->GetString("command");
	ExecuteCommand(LastCommand);
}

void OnCommandClose()
{
	delete(CommandDialog);
	CommandDialog = NULL;
}

void ExecuteCommandDialog()
{
	msg_db_r("ExecCmdDlg", 1);
	if (!CommandDialog){
		CommandDialog = HuiCreateResourceDialog("command_dialog", MainWin);
		CommandDialog->SetString("command", LastCommand);
		CommandDialog->Event("ok", &OnCommandOk);
		CommandDialog->Event("cancel", &OnCommandClose);
		CommandDialog->Event("hui:close", &OnCommandClose);
		CommandDialog->Update();
	}

	HuiWaitTillWindowClosed(CommandDialog);
	msg_db_l(1);
}
bool change_return = true;

void insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, gpointer user_data)
{
	if (!HistoryEnabled)
		return;

	msg_db_r("insert_text", 1);

	if ((strcmp(text, "\n") == 0) && (change_return)){
		g_signal_stop_emission_by_name(textbuffer, "insert-text");
		change_return = false;
		InsertReturn();
		change_return = true;
		msg_db_l(1);
		return;
	}

	char *text2 = (char*)g_malloc(len + 1);
	memcpy(text2, text, len);
	ChangeHistory(UndoInsert, text2, len, gtk_text_iter_get_offset(location));
	SetWindowTitle();

	NeedsUpdateStart = gtk_text_iter_get_line(location);
	NeedsUpdateEnd = NeedsUpdateStart;
	for (int i=0;i<len;i++)
		if (text[i] == '\n')
			NeedsUpdateEnd ++;
	msg_db_l(1);
}

void delete_range(GtkTextBuffer *textbuffer, GtkTextIter *start, GtkTextIter *end, gpointer user_data)
{
	if (!HistoryEnabled)
		return;
	msg_db_r("delete_range", 1);
	char *text = gtk_text_buffer_get_text(textbuffer, start, end, false);
	ChangeHistory(UndoRemove, text, strlen(text), gtk_text_iter_get_offset(start));
	SetWindowTitle();

	NeedsUpdateStart = gtk_text_iter_get_line(start);
	NeedsUpdateEnd = NeedsUpdateStart;
	msg_db_l(1);
}

void move_cursor(GtkTextView *text_view, GtkMovementStep step, gint count, gboolean extend_selection, gpointer user_data)
{
	msg_db_r("move_cursor", 1);
	if (step == GTK_MOVEMENT_DISPLAY_LINE_ENDS){
		g_signal_stop_emission_by_name(text_view, "move-cursor");
		if (count > 0)
			JumpToEndOfLine(extend_selection);
		else
			JumpToStartOfLine(extend_selection);
	}
	//printf("move cursor  %d  %d  %d\n", count, (int)extend_selection, (int)step);
	msg_db_l(1);
}

void copy_clipboard(GtkTextView *text_view, gpointer user_data)
{	msg_db_r("copy_cb", 1);	g_signal_stop_emission_by_name(text_view, "copy-clipboard");	msg_db_l(1);	}

void paste_clipboard(GtkTextView *text_view, gpointer user_data)
{	msg_db_r("paste_cb", 1);	g_signal_stop_emission_by_name(text_view, "paste-clipboard");	msg_db_l(1);	}

void cut_clipboard(GtkTextView *text_view, gpointer user_data)
{	msg_db_r("cut_cb", 1);	g_signal_stop_emission_by_name(text_view, "cut-clipboard");	msg_db_l(1);	}

void toggle_cursor_visible(GtkTextView *text_view, gpointer user_data)
{	msg_db_r("toggle_cursor_visible", 1);	g_signal_stop_emission_by_name(text_view, "toggle-cursor-visible");	msg_db_l(1);	}

void ShowLineOnScreen(int line)
{
	msg_db_r("ShowLineOnScreen", 1);
	GtkTextIter it;
	gtk_text_buffer_get_iter_at_line(tb, &it, line);
	gtk_text_buffer_place_cursor(tb, &it);
	//gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb)); // line is minimally visible
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(tv), &it, false, true, 0.0, 0.5); // line is vertically centered
	msg_db_l(1);
}

gboolean CallbackJumpLine(GtkWidget *widget, gpointer data)
{
	msg_db_r("CallbackJumpLine", 1);
	int line = (int)(long)data;
	ShowLineOnScreen(line);
	//msg_write((int)(long)data);
	msg_db_l(1);
	return FALSE;
}

void populate_popup(GtkTextView *text_view, GtkMenu *menu, gpointer user_data)
{
	msg_db_r("populate_popup", 1);
	if (Filename.extension() != "kaba"){
		msg_db_l(1);
		return;
	}
	FindScriptFunctions();
	GtkWidget *m = gtk_separator_menu_item_new();
	gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), m);
	gtk_widget_show(m);
	if (ScriptFunction.num == 0){
		m = gtk_menu_item_new_with_label(_("- Keine Funktionen -").c_str());
		gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), m);
		gtk_widget_set_sensitive(m, false);
		gtk_widget_show(m);
	}
	foreachb(sScriptFunction &f, ScriptFunction){
		m = gtk_menu_item_new_with_label(f.name.c_str());
		gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), m);
		gtk_widget_show(m);
		g_signal_connect(G_OBJECT(m), "activate", G_CALLBACK(CallbackJumpLine), (void*)(long)f.line);
	}
	msg_db_l(1);
}

int color_busy_level = 0;

void CreateColorsIfNotBusy()
{
	color_busy_level --;
	if (color_busy_level == 0)
		CreateTextColors();
}

void changed(GtkTextBuffer *textbuffer, gpointer user_data)
{
	msg_db_r("changed", 1);
	//printf("change\n");
	CreateTextColors(NeedsUpdateStart, NeedsUpdateEnd);
	HuiRunLater(3000, &CreateColorsIfNotBusy);
	color_busy_level ++;
	msg_db_l(1);
}

void OnAbout()
{	HuiAboutBox(MainWin);	}

void OnConsoleClose()
{	MainWin->HideControl("table_console", true);	}

void OnExit()
{
	if (AllowTermination()){
		irect r=MainWin->GetOuteriorDesired();
		HuiConfigWriteInt("Width",r.x2-r.x1);
		HuiConfigWriteInt("Height",r.y2-r.y1);
		HuiConfigWriteInt("X",r.x1);
		HuiConfigWriteInt("Y",r.y1);
		HuiConfigWriteBool("Maximized",MainWin->IsMaximized());
		HuiEnd();
		msg_db_l(1);
	}
}


void SetTag(int i, const char *fg_color, const char *bg_color, bool bold, bool italic)
{
	tag[i].tag = gtk_text_buffer_create_tag(tb, NULL, "foreground", fg_color, NULL);
	if (bg_color)
		g_object_set(tag[i].tag, "background", bg_color, NULL);
	if (bold)
		g_object_set(tag[i].tag, "weight", PANGO_WEIGHT_BOLD, NULL);
	if (italic)
		g_object_set(tag[i].tag, "style", PANGO_STYLE_ITALIC, NULL);
}

int hui_main(Array<string> arg)
{
	msg_init(false);
	msg_db_r("main",1);
	HuiInitExtended("sgribthmaker", AppVersion, NULL, true, "Deutsch");
	msg_init(HuiAppDirectory + "message.txt", ALLOW_LOGGING);

	HuiSetProperty("name", AppTitle);
	HuiSetProperty("version", AppVersion);
	HuiSetProperty("comment", _("Texteditor und Kaba-Compiler"));
	HuiSetProperty("website", "http://michisoft.michi.is-a-geek.org");
	HuiSetProperty("copyright", "© 2006-2012 by MichiSoft TM");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");

	timer = HuiCreateTimer();
	CompileTimer = HuiCreateTimer();

	HuiGetTime(timer);

	HuiRegisterFileType("kaba","MichiSoft Script Datei",HuiAppDirectory + "Data/kaba.ico",HuiAppFilename,"open",true);

	int width = HuiConfigReadInt("Width", 800);
	int height = HuiConfigReadInt("Height", 600);
	int x = HuiConfigReadInt("X", 0);
	int y = HuiConfigReadInt("Y", 0);
	bool maximized = HuiConfigReadBool("Maximized", false);
	int font_size = HuiConfigReadInt("FontSize", 8);

	HuiAddCommand("new", "hui:new", KEY_N + KEY_CONTROL, &New);
	//HuiAddKeyCode(HMM_NEW_HEX, KEY_F1 + 256);
	HuiAddCommand("open", "hui:open", KEY_O + KEY_CONTROL, &OnOpen);
	//HuiAddKeyCode(HMM_OPEN_HEX, KEY_F9 + 256);
	HuiAddCommand("save", "hui:save", KEY_S + KEY_CONTROL, &OnSave);
	HuiAddCommand("save_as", "hui:save_as", KEY_S + KEY_SHIFT + KEY_CONTROL, &OnSaveAs);
	HuiAddCommand("exit", "hui:quit", KEY_Q + KEY_CONTROL, &OnExit);
	//HuiAddCommand("show_data", "", KEY_D + KEY_CONTROL, &ShowData);
	HuiAddCommand("execute_command", "", KEY_E + KEY_CONTROL, &ExecuteCommandDialog);
	HuiAddCommand("find", "", KEY_F + KEY_CONTROL, &ExecuteCommandDialog);
	HuiAddCommand("cut", "hui:cut", KEY_X + KEY_CONTROL, &Cut);
	HuiAddCommand("copy", "hui:copy", KEY_C + KEY_CONTROL, &CopyToClipboard);
	HuiAddCommand("paste", "hui:paste", KEY_V + KEY_CONTROL, &PasteFromClipboard);
	HuiAddCommand("reload", "hui:reload", KEY_R + KEY_CONTROL, &OnReload);
	HuiAddCommand("undo", "hui:undo", KEY_Z + KEY_CONTROL, &Undo);
	HuiAddCommand("redo", "hui:redo", KEY_Y + KEY_CONTROL, &Redo);
	HuiAddKeyCode("redo", KEY_Z + KEY_SHIFT + KEY_CONTROL);
	HuiAddCommand("compile", "", KEY_F7, &Compile);
	HuiAddCommand("compile_and_run_verbose", "", KEY_F6 + KEY_CONTROL, &OnCompileAndRunVerbose);
	HuiAddCommand("compile_and_run", "", KEY_F6, &OnCompileAndRunSilent);
	//HuiAddCommand("script_help", "hui:help", KEY_F1 + KEY_SHIFT);
	
	HuiAddCommand("show_cur_line", "", KEY_F2, &ShowCurLine);

	MainWin = HuiCreateControlWindow(AppTitle, -1, -1, width, height);

	MainWin->AllowEvents("key");

	MainWin->Event("about", &OnAbout);
	MainWin->Event("console_close", &OnConsoleClose);
	MainWin->Event("exit", &OnExit);
	MainWin->Event("hui:close", &OnExit);


	MainWin->SetBorderWidth(0);
	MainWin->AddControlTable("", 0, 0, 1, 2, "table_main");
	MainWin->SetTarget("table_main", 0);
	MainWin->AddMultilineEdit("", 0, 0, 0, 0, "edit");
	MainWin->AddControlTable("", 0, 1, 1, 2, "table_console");
	MainWin->SetTarget("table_console", 0);
	MainWin->AddMultilineEdit("", 0, 0, 0, 0, "log");
	MainWin->Enable("log", false);
	MainWin->AddControlTable("!noexpandy", 0, 1, 3, 1, "table_command");
	MainWin->SetTarget("table_command", 0);
	MainWin->AddEdit("", 0, 0, 0, 0, "console");
	MainWin->AddButton("!noexpandx\\Ok", 1, 0, 0, 0, "console_ok");
	MainWin->SetImage("console_ok", "hui:ok");
	MainWin->AddButton("", 2, 0, 0, 0, "console_close");
	MainWin->SetImage("console_close", "hui:close");
	MainWin->HideControl("table_console", true);
	
	tv = MainWin->_GetControl_("edit")->widget;
	tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));

	g_signal_connect(G_OBJECT(tb),"insert-text",G_CALLBACK(insert_text),NULL);
	g_signal_connect(G_OBJECT(tb),"delete-range",G_CALLBACK(delete_range),NULL);
	g_signal_connect(G_OBJECT(tb),"changed",G_CALLBACK(changed),NULL);

	g_signal_connect(G_OBJECT(tv),"move-cursor",G_CALLBACK(move_cursor),NULL);
	g_signal_connect(G_OBJECT(tv),"copy-clipboard",G_CALLBACK(copy_clipboard),NULL);
	g_signal_connect(G_OBJECT(tv),"paste-clipboard",G_CALLBACK(paste_clipboard),NULL);
	g_signal_connect(G_OBJECT(tv),"cut-clipboard",G_CALLBACK(cut_clipboard),NULL);
	g_signal_connect(G_OBJECT(tv),"toggle-cursor-visible",G_CALLBACK(toggle_cursor_visible),NULL);
	g_signal_connect(G_OBJECT(tv),"populate-popup",G_CALLBACK(populate_popup),NULL);


	/* Change default font throughout the widget */
	PangoFontDescription *font_desc;
	font_desc = pango_font_description_from_string(format("Monospace %d", font_size).c_str());
	gtk_widget_modify_font(tv, font_desc);
	pango_font_description_free(font_desc);
	GdkColor color;
	if (COLORMODE == 1)
		gdk_color_parse ("#ffffff", &color);
	else{
		gdk_color_parse ("#000000", &color);
		gtk_widget_modify_base(tv, GTK_STATE_NORMAL, &color);
	}
	//g_object_set(tv, "wrap-mode", GTK_WRAP_WORD_CHAR, NULL);


	// Tab size
	int tab_size = HuiConfigReadInt("TabWidth", 8);
	PangoLayout *layout = gtk_widget_create_pango_layout(tv, " ");
	pango_layout_get_pixel_size(layout, &width, &height);
	PangoTabArray *ta = pango_tab_array_new(1, true);
	pango_tab_array_set_tab(ta, 0, PANGO_TAB_LEFT, width * tab_size);
	gtk_text_view_set_tabs(GTK_TEXT_VIEW(tv), ta);


	if (COLORMODE == 1){
		SetTag(InLineComment, "#808080", NULL, false, true);
		SetTag(InCommentLevel1, "#808080", NULL, false, true);
		SetTag(InCommentLevel2, "#b0b0b0", NULL, false, true);
		SetTag(InSpace, "#000000", NULL, false, false);
		SetTag(InWord, "#000000", NULL, false, false);
		SetTag(InWordType, "#2000d0", NULL, true, false);
		SetTag(InWordGameVariable, "#a0a000", NULL, false, false);
		SetTag(InWordCompilerFunction, "#1000a0", NULL, false, false);
		SetTag(InWordSpecial, "#a000a0", NULL, true, false);
		SetTag(InNumber, "#008000", NULL, false, false);
		SetTag(InOperator, "#404000", NULL, false, false);
		SetTag(InString, "#ff0000", NULL, false, false);
		SetTag(InMacro, "#008080", NULL, false, false);
		SetTag(InSpecialCallback, "#1000a0", NULL, true, false);
	}else{
		/*SetTag(InLineComment, "#808080", NULL, false, false);
		SetTag(InCommentLevel1, "#808080", NULL, false, false);
		SetTag(InCommentLevel2, "#b0b0b0", NULL, false, false);
		SetTag(InSpace, "#000000", NULL, false, false);
		SetTag(InWord, "#000000", NULL, false, false);
		SetTag(InWordType, "#2000d0", NULL, false, false);
		SetTag(InWordGameVariable, "#a0a000", NULL, false, false);
		SetTag(InWordCompilerFunction, "#1000a0", NULL, false, false);
		SetTag(InWordSpecial, "#a000a0", NULL, false, false);
		SetTag(InNumber, "#008000", NULL, false, false);
		SetTag(InOperator, "#404000", NULL, false, false);
		SetTag(InString, "#ff0000", NULL, false, false);
		SetTag(InMacro, "#008080", NULL, false, false);
		tag[InLineComment] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#808080", NULL);
		tag[InCommentLevel1] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#808080", NULL);
		tag[InCommentLevel2] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#b0b0b0", NULL);
		tag[InSpace] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#000000", NULL);
		tag[InWord] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#ffffff", NULL);
		tag[InWordType] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#6040ff", NULL);
		tag[InWordGameVariable] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#b0b000", NULL);
		tag[InWordCompilerFunction] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#5040ff", NULL);
		tag[InWordSpecial] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#ff40ff", NULL);
		tag[InNumber] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#40a040", NULL);
		tag[InOperator] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#ffffb0", NULL);
		tag[InString] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#ff0000", NULL);
		tag[InMacro] = gtk_text_buffer_create_tag(tb, NULL, "foreground", "#008080", NULL);*/
	}

	MainWin->SetMenu(HuiCreateResourceMenu("menu"));
	MainWin->SetMaximized(maximized);
	MainWin->Update();

	Script::Init();

	New();

	if (arg.num > 1)
		LoadFromFile(arg[1]);

	return HuiRun();
}

