#include <iostream>
#include <vector>
#include <fstream>
#include <windows.h>
#include <ctime>
#include <limits>
#include <iomanip>
#include <ctype.h>
#include <sstream>

using namespace std;

struct subtask
{
    string name;
    bool important;
    bool finished;
    bool fixedLen;
    int len;
};

struct task
{
    vector <subtask> Sub;
    string name;
    int len;
    int bgn;
    int end;
    bool finished;
};

struct day
{
    vector <task> Tasks;
    vector <string> Notes;
    bool allTasksFinished;
    int wDay; //used only in template
};

struct current
{
    int wDay;
    int mDay;
    int yr;
    int mo;
};


void setAsDone(vector <task> &Tasks, string gap);
void selectTask(vector <task> Tasks, string gap, int &t, int &subT, string &input);
void addTask(vector <vector <day> > &Months, int m, int d, string gap);
void addSubtask(vector <task> &Tasks, string gap);
void removeTask(vector <task> &Tasks, string gap);
void getDates(struct current &cr);
void dateToToday(int &actvMo, int &actvDay, struct current cr, string MonthsName[]);
void dateToTomorrow(int &actvMo, int &actvDay, string MonthsName[], vector <vector <day> > Months);
void changeDate(int &actvMo, int &actvDay, string MonthsName[], vector <vector <day> > Months);
void outputNumberedTasks(vector <task> Tasks, string gap);
void outputActiveDay(struct current cr, int actvMo, int actvDay, vector <vector <day> > Months, string MonthsName[], string WDays[], string gap, bool templateSet);
void saveToFile(int y, vector <vector <day> > Months);
void readFromFile(vector <vector <day> > &Months);
void outputAmount(int num);
int subtaskLength(vector <subtask> Sub);
int inputHoursMinutes(string inputType, int lowerLimit, int upperLimit);
int dayOfWeek(int y, int m, int d);
int numberOfDays(int month, int year);
int readLength(ifstream &in);
bool readBool(ifstream &in, string criteria);
bool tasksFinished(vector <task> Tasks);
subtask getSubtask(struct task mainTask);
string hoursAndMinutes(int len);

int main() //save atexit function
{
    string WDays[8] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    string MonthsName[13] = {"blank", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    string gap = "-------------------------------------\n";
    string option;
    int defaultYear = 2016, actvMo, actvDay; //TODO: adapt for several years; change seperate actv variables to current struct
    struct current cr;
    bool templateSet = false; //current
    getDates(cr);
    actvMo = cr.mo;
    actvDay = cr.mDay;
    if (cr.yr != defaultYear) //current workaround
    {
        cout << "Sorry! The program currently only works in " << defaultYear << endl;
        return 0;
    }
    vector <vector <day> > Months(13);
    for (int i = 1; i <= 12; i++)
        Months[i].resize(numberOfDays(i, cr.yr) + 1);
    readFromFile(Months);/* current
    if (Months[0][0].Tasks.size() == 0)
        templateSet = false;
    else templateSet = true; */
    while (true)
    {
        outputActiveDay(cr, actvMo, actvDay, Months, MonthsName, WDays, gap, templateSet);
        cout << "[1]  Change active date" << endl;
        cout << "[2]  Change active date back to today" << endl;
        cout << "[3]  Change active date to tomorrow" << endl;
        cout << "[4]  Set task/subtask on active date as completed" << endl;
        cout << "[5]  Add new task to the active date" << endl;
        cout << "[6]  Add a subtask" << endl;
        cout << "[7]  Remove task/subtask" << endl;
        cout << "[8]  Set schedule template" << endl; //weekly? dedicate [0][0] to a special template?
        cout << "[9]  Output the active week's schedule" << endl;
        cout << "[10] Transfer previously unfinished tasks" << endl; //all, this week's, the previous week's
        cout << "[11] Add a note to the active date" << endl;
        cout << "[12] Remove a note from the active date" << endl;//print first few words when outputing selection
        cout << "[13] Re-read data file (WARNING: current changes will be lost)" << endl;
        cout << "[14] Save and exit" << endl;
        cout << "[15] Exit without saving" << endl;
        cout << gap;
        cout << "Select menu option: ";
        //while (go)
            cin >> option;
        if (option == "1")
            changeDate(actvMo, actvDay, MonthsName, Months);
        else if (option == "2")
            dateToToday(actvMo, actvDay, cr, MonthsName);
        else if (option == "3")
            dateToTomorrow(actvMo, actvDay, MonthsName, Months);
        else if (option == "4")
            setAsDone(Months[actvMo][actvDay].Tasks, gap);
        else if (option == "5")
            addTask(Months, actvMo, actvDay, gap);
        else if (option == "6")
            addSubtask(Months[actvMo][actvDay].Tasks, gap);
        else if (option == "7")
            removeTask(Months[actvMo][actvDay].Tasks, gap);
        else if (option == "13")
            readFromFile(Months);
        else if (option == "14")
        {
            saveToFile(cr.yr, Months);
            break;
        }
        else if (option == "15")
            break;
        else
        {
            system("cls");
            cout << "Select a valid option!" << endl;
        }
    }
    return 0;
}

bool tasksFinished(vector <task> Tasks)
{
    for (int i = 0; i < Tasks.size(); i++)
        if (!Tasks[i].finished)
            return false;
    return true;
}

void saveToFile(int y, vector <vector <day> > Months)
{
    int daysWithThings = 0;
    string Imp[2] = {"Regular", "Important"};
    string Compl[2] = {"Uncompleted", "Completed"};
    string Fixed[2] = {"Not fixed", "Fixed"};
    ofstream out("data.csv");
    bool pr;
    for (int i = 1; i <= 12; i++)
        for (int j = 1; j < Months[i].size(); j++)
        {
            if (Months[i][j].Tasks.size() > 0 || Months[i][j].Notes.size() > 0)
            {
                pr = true;
                daysWithThings++;
                out << y << '-' << i << '-' << j << ';' << Months[i][j].Tasks.size() << " task(s);" << Months[i][j].Notes.size() << " note(s)" << endl;
            }
            if (Months[i][j].Tasks.size() > 0)
            {
                vector <task> Tasks;
                Tasks = Months[i][j].Tasks;
                for (int k = 0; k < Tasks.size(); k++)
                {
                    out << Tasks[k].name << ';' << hoursAndMinutes(Tasks[k].bgn) << ';' << hoursAndMinutes(Tasks[k].end) << ';' << Compl[Tasks[k].finished] << ';' << Tasks[k].Sub.size() << " subt." << endl;
                    for (int l = 0; l < Tasks[k].Sub.size(); l++)
                    {
                        bool fix = Tasks[k].Sub[l].fixedLen;
                        bool imp = Tasks[k].Sub[l].important;
                        bool fin = Tasks[k].Sub[l].finished;
                        out << Tasks[k].Sub[l].name << ';' << Imp[imp] << ';' << Compl[fin] << ';' << Fixed[fix] << ';';
                        if(fix)
                            out << hoursAndMinutes(Tasks[k].Sub[l].len);
                        out << endl;
                    }
                }
            }
            if (Months[i][j].Notes.size() > 0)
            {
                //TODO
            }
            if (pr)
                out << endl;
        }
    outputAmount(daysWithThings);
}

void outputAmount(int num)
{
    ifstream in("data.csv");
    stringstream buffer;
    buffer << in.rdbuf();
    ofstream out("data.csv");
    out << num << endl;
    out << buffer.str() << endl << endl;
}

void readFromFile(vector <vector <day> > &Months)
{
    int m, d, days,
    numb;
    string data;
    ifstream in("data.csv");
    if(!in.is_open())
        cout << "Failed to open data file!" << endl;
    else
    {
        in >> days;
        for (int n = 0; n < days; n++)
        {
            in.ignore(256, '-');
            in >> m;
            in.ignore();
            in >> d;
            in.ignore();
            in >> numb;
            Months[m][d].Tasks.resize(numb);
            in.ignore(256, ';');
            in >> numb;
            Months[m][d].Notes.resize(numb);
            in.ignore(256, '\n');
            for (int i = 0; i < Months[m][d].Tasks.size(); i++)
            {
                task tmp;
                getline(in, data, ';');
                tmp.name = data;
                tmp.bgn = readLength(in);
                tmp.end = readLength(in);
                tmp.len = tmp.end - tmp.bgn;
                tmp.finished = readBool(in, "Completed");
                in >> numb;
                tmp.Sub.resize(numb);
                in.ignore(256, '\n');
                for (int j = 0; j < tmp.Sub.size(); j++)
                {
                    subtask tmpSub;
                    getline(in, data, ';');
                    tmpSub.name = data;
                    tmpSub.important = readBool(in, "Important");
                    tmpSub.finished = readBool(in, "Completed");
                    tmpSub.fixedLen = readBool(in, "Fixed");
                    if (tmpSub.fixedLen)
                        tmpSub.len = readLength(in);
                    in.ignore();
                    tmp.Sub[j] = tmpSub;
                }
                Months[m][d].Tasks[i] = tmp;
            }
            for (int i = 0; i < Months[m][d].Notes.size(); i++)
            {
                //TODO
            }
        }
        if (in.good())
            cout << "Data file successfully loaded!" << endl;
    }
}

bool readBool(ifstream &in, string criteria)
{
    string data;
    getline(in, data, ';');
    if (data == criteria)
        return true;
    else
        return false;
}

int readLength(ifstream &in)
{
    int h, m;
    in >> h;
    in.ignore();
    in >> m;
    in.ignore();
    return ((h * 60) + m);
}

string hoursAndMinutes(int len)
{
    stringstream ss;
    int hm = len / 60;
    if (hm < 10)
        ss << '0';
    ss << hm << ':';
    hm = len % 60;
    if (hm < 10)
        ss << '0';
    ss << hm;
    return ss.str();
}

void outputActiveDay(struct current cr, int actvMo, int actvDay, vector <vector <day> > Months, string MonthsName[], string WDays[], string gap, bool templateSet)
{
    string status = "-+";
    cout << gap << WDays[dayOfWeek(cr.yr, actvMo, actvDay)] << ", " << MonthsName[actvMo] << " " << actvDay << endl << gap;
    if (actvMo == 0 && actvDay == 0)
        cout << "TEMPLATE CREATION MODE" << endl;
    else
        if (Months[actvMo][actvDay].Tasks.size() == 0)
            if (!templateSet)
                cout << "No tasks for today so far - add some!" << endl << gap;
    else
    {
        vector <task> tmp(Months[actvMo][actvDay].Tasks);
        for (int i = 0; i < Months[actvMo][actvDay].Tasks.size(); i++)
        {
            cout << '[' << status[tmp[i].finished] << "]  " << tmp[i].name << " ";
            cout << hoursAndMinutes(tmp[i].bgn) << " - " << hoursAndMinutes(tmp[i].end) << endl;
            for (int j = 0; j < tmp[i].Sub.size(); j++)
            {
                cout << "     [" << status[tmp[i].Sub[j].finished] << ']';
                if (tmp[i].Sub[j].important)
                    cout << " !!! ";
                else
                    cout << "     ";
                cout << tmp[i].Sub[j].name << " ";
                if (tmp[i].Sub[j].fixedLen)
                    cout << hoursAndMinutes(tmp[i].Sub[j].len);
                cout << endl;
            }
        }
        cout << gap;
    }
}

void outputNumberedTasks(vector <task> Tasks, string gap)
{
    system("cls");
    cout << endl << gap;
    for (int i = 0; i < Tasks.size(); i++)
    {
        cout << "[" << i + 1 << "] " << Tasks[i].name << endl;
        for (int j = 0; j < Tasks[i].Sub.size(); j++)
            cout << "    [" << (char) (j + 97) << "] " << Tasks[i].Sub[j].name << endl;
    }
}

void setAsDone(vector <task> &Tasks, string gap)
{
    string input;
    int t, subT;
    if (Tasks.size() > 0)
        outputNumberedTasks(Tasks, gap);
    else
    {
        system("cls");
        cout << "No tasks found!" << endl;
    }
    while (true && Tasks.size() > 0)
    {
        selectTask(Tasks, gap, t, subT, input);
        if (input.length() == 2)
            if (!Tasks[t].Sub[subT].finished)
                Tasks[t].Sub[subT].finished = true;
            else
                cout << "The selected subtask is already set as completed!" << endl;
        else
            if (!Tasks[t].finished)
            {
                Tasks[t].finished = true;
                for (int i = 0; i < Tasks[t].Sub.size(); i++)
                Tasks[t].Sub[i].finished = true;
            }
            else
                cout << "The selected task is already set as completed!" << endl;
        cout << gap << "Select another? Y for yes, anything else for no ";
        cin >> input;
        if (input != "Y" && input != "y")
        {
            system("cls");
            cout << endl;
            break;
        }
    }
}

void selectTask(vector <task> Tasks, string gap, int &t, int &subT, string &input)
{
    while (true)
    {
        cout << gap << "Select task/subtask (e. g. 1a for subtask): ";
        cin >> input;
        if (input.length() == 2 && isdigit(input[0]) && isalpha(input[1]))
        {
            t = (int) input[0] - 49;
            subT = (int) input[1] - 97;
            if (input.length() == 2 && t >= 0 && t < Tasks.size() && subT >= 0 && subT < Tasks[t].Sub.size())
                break;
            else
                cout << "The selected subtask does not exist!" << endl;

        }
        else if (input.length() == 1 && isdigit(input[0]))
        {
            t = (int) input[0] - 49;
            if (input.length() == 1 && t >= 0 && t < Tasks.size())
                break;
            else
                cout << "The selected task does not exist!" << endl;
        }
        else
            cout << "Wrong input!" << endl;
    }
}

void addTask(vector <vector <day> > &Months, int m, int d, string gap) //TODO: check for conflicting times (optional?); fix if finish time exceeds 24hrs
{
    system("cls");
    cout << endl << gap;
    string option;
    while (true)
    {
        task tmp;
        tmp.finished = false;
        cout << "Enter task name: ";
        cin.ignore();
        getline(cin, tmp.name);
        tmp.bgn = inputHoursMinutes("start time", -1, -1);
        tmp.len = inputHoursMinutes("length", 0, -1);
        tmp.end = tmp.bgn + tmp.len;
        cout << gap;
        cout << "Do you want to add subtasks? Y for yes, any other string for no ";
        cin >> option;
        if (option == "Y" || option == "y")
        {
            cout << gap;
            while(true)
            {
                tmp.Sub.push_back(getSubtask(tmp));
                cout << gap << "Add more subtasks? Y for yes, any other string for no ";
                cin >> option;
                cout << gap;
                if (option != "Y" && option != "y")
                    break;
            }
        }
        Months[m][d].Tasks.push_back(tmp);
        cout << "Do you want to add more tasks? Y for yes, any other string for no ";
        cin >> option;
        if (option != "Y" && option != "y")
        {
            system("cls");
            cout << endl;
            break;
        }
        else
            cout << gap;
    }
}

void addSubtask(vector <task> &Tasks, string gap)
{
    string input;
    int t, subT;
    if (Tasks.size() > 0)
        outputNumberedTasks(Tasks, gap);
    else
    {
        system("cls");
        cout << "No tasks found!" << endl;
    }
    while (true && Tasks.size() != 0)
    {
        selectTask(Tasks, gap, t, subT, input);
        if (input.length() == 1)
            Tasks[t].Sub.push_back(getSubtask(Tasks[t]));
        else
            cout << "Subtasks can't have subtasks!" << endl;
        cout << gap << "Select again? Y for yes, anything else for no ";
        cin >> input;
        if (input != "Y" && input != "y")
        {
            system("cls");
            cout << endl;
            break;
        }
    }
}

void removeTask(vector <task> &Tasks, string gap)
{
    string input;
    int t, subT;
    if (Tasks.size() == 0)
    {
        system("cls");
        cout << "No tasks found!" << endl;
    }
    while (true && Tasks.size() != 0)
    {
        outputNumberedTasks(Tasks, gap);
        selectTask(Tasks, gap, t, subT, input);
        if (input.length() == 2)
            Tasks[t].Sub.erase(Tasks[t].Sub.begin() + subT);
        else
            Tasks.erase(Tasks.begin() + t);
        if (Tasks.size() > 0)
        {
            cout << gap << "Select again? Y for yes, anything else for no ";
            cin >> input;
            if (input != "Y" && input != "y")
            {
                system("cls");
                cout << endl;
                break;
            }
        }
    }
}

int subtaskLength(vector <subtask> Sub)
{
    int sum = 0;
    for (int i = 0; i < Sub.size(); i++)
        if (Sub[i].fixedLen)
            sum += Sub[i].len;
    return sum;
}

int inputHoursMinutes(string inputType, int lowerLimit, int upperLimit)
{
    if (lowerLimit == -1)
        lowerLimit = numeric_limits <int> ::min();
    if (upperLimit == -1)
        upperLimit = numeric_limits <int> ::max();
    while(true)
    {
        int h, min;
        char colon;
        cout << "Enter " << inputType << " in HH:MM format: ";
        cin >> h >> colon >> min;
        if (cin.good() && h >= 0 && h < 24 && min < 60 && min >= 0 && (h * 60 + min) > lowerLimit && (h * 60 + min) < upperLimit)
            return (h * 60 + min);
        else if ((h * 60 + min) > upperLimit)
        {
            cout << "Too long!" << endl;
            cin.clear();
            cin.ignore(numeric_limits <int> ::max(), '\n');
        }
        else
        {
            cout << "Wrong input!" << endl;
            cin.clear();
            cin.ignore(numeric_limits <int> ::max(), '\n');
        }
    }
}

void getDates(struct current &cr)
{
    time_t t = time(0);
	struct tm* now = localtime(&t);
	cr.yr = 1900 + now -> tm_year;
	cr.mo = now -> tm_mon + 1;
	cr.mDay = now -> tm_mday;
	cr.wDay = now -> tm_wday;
}

void dateToToday(int &actvMo, int &actvDay, struct current cr, string MonthsName[])
{
    actvMo = cr.mo;
    actvDay = cr.mDay;
    system("cls");
    cout << "Active date successfully changed to " << MonthsName[actvMo] << " " << actvDay << endl;
}

void dateToTomorrow(int &actvMo, int &actvDay, string MonthsName[], vector <vector <day> > Months)
{
    if (actvDay + 1 < Months[actvMo].size())
        actvDay++;
    else
    {
        actvMo++;
        actvDay = 1;
    }
    system("cls");
    cout << "Active date successfully changed to " << MonthsName[actvMo] << " " << actvDay << endl;
}

void changeDate(int &actvMo, int &actvDay, string MonthsName[], vector <vector <day> > Months)
{
    while (true)
    {
        cout << "Enter month number: ";
        cin >> actvMo;
        if (cin.good() && (actvMo > 0 && actvMo <= 12))
            break;
        else
        {
            cin.clear();
            cin.ignore(numeric_limits <int> ::max(), '\n');
            cout << "Not a valid month!" << endl;
        }
    }
    cin.ignore(numeric_limits <int> ::max(), '\n');
    while (true)
    {
        cout << "Enter day of month: ";
        cin >> actvDay;
        if (cin.good() && (actvDay > 0 && actvDay <= Months[actvMo].size()))
            break;
        else
        {
            cin.clear();
            cin.ignore(numeric_limits <int> ::max(), '\n');
            cout << "Not a valid day!" << endl;
        }
    }
    system("cls");
    cout << "Active date successfully changed to " << MonthsName[actvMo] << " " << actvDay << endl;
}

int numberOfDays(int month, int year)
{
    int no;
    if (month == 4 || month == 6 || month == 9 || month == 11)
        no = 30;
    else
        if (month == 2)
        {
            if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) //check if leap year
                no = 29;
            else
                no = 28;
        }
        else
            no = 31;
    return no;
}

int dayOfWeek(int y, int m, int d)
{
    int T[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    y -= m < 3;
    return (y + y/4 - y/100 + y/400 + T[m-1] + d) % 7;
}

subtask getSubtask(struct task mainTask)
{
    string option;
    subtask tmpSub;
    tmpSub.finished = false;
    cout << "Enter subtask name: ";
    cin.ignore();
    getline(cin, tmpSub.name);
    cout << "Do you want to set this subtask as important? Y for yes, any other string for no ";
    cin >> option;
    if (option == "Y" || option == "y")
        tmpSub.important = true;
    else
        tmpSub.important = false;
    cout << "Do you want to set a fixed time for this subtask? Y for yes, any other string for no ";
    cin >> option;
    if (option == "Y" || option == "y")
        tmpSub.fixedLen = true;
    else
        tmpSub.fixedLen = false;
    if (tmpSub.fixedLen)
        tmpSub.len = inputHoursMinutes("length", -1, mainTask.len - subtaskLength(mainTask.Sub));
    return tmpSub;
}
