function varargout = nips_demo(varargin)
% NIPS_DEMO M-filemenu for nips_demo.fig
%      NIPS_DEMO, by itself, creates a new NIPS_DEMO or raises the existing
%      singleton*.
%
%      H = NIPS_DEMO returns the handle to a new NIPS_DEMO or the handle to
%      the existing singleton*.
%
%      NIPS_DEMO('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in NIPS_DEMO.M with the given input arguments.
%
%      NIPS_DEMO('Property','Value',...) creates a new NIPS_DEMO or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before nips_demo_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to nips_demo_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help nips_demo

% Last Modified by GUIDE v2.5 03-Dec-2005 23:03:32


% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @nips_demo_OpeningFcn, ...
                   'gui_OutputFcn',  @nips_demo_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


function newhandles=resetView(handles)
% Stops playback and updates the view with the newly loaded problem.
%stopPlayback;
set(handles.TimeSlider, 'Min', 1);
set(handles.TimeSlider, 'Max', handles.prob.nSteps);
handles.step = 1;
newhandles=handles;
updateView(handles, 1);

function newhandles=setView(handles, view)
viewPanel=[handles.ActionPanel handles.AnimationOptionsPanel handles.GraphsPanel];
if view~=handles.view
  if handles.view
    set(viewPanel(handles.view), 'Visible', 'off');
  end
  handles.view=view;
  set(viewPanel(handles.view), 'Visible', 'on');
  updateView(handles, 0);
end
newhandles=handles;

function updateView(handles, updateSlider)
if isfield(handles, 'prob')
  step = round(handles.step);
  if exist('updateSlider', 'var') && updateSlider
    set(handles.TimeSlider, 'Value', handles.step);
  end
  prob = handles.prob;
  if handles.playing == 1
    status = 'Playing';
  else
    status = 'Stopped';
  end
  set(handles.StatusBar, 'String', ...
    [status ': ' time2str(prob.time(step)) '/' time2str(prob.time(end)) ...
     ', step ' num2str(step) '/' num2str(prob.nSteps)]);
  switch handles.view
    case 1      % Data view
      plotSlatObservation(prob, step);
    case 2      % Solution view
      subplot(1,1,1);
      if get(handles.ViewKFButton, 'Value')==1,
        soln = handles.kf;
      elseif get(handles.ViewDBKButton, 'Value')==1,
        soln = handles.dbk;
      end
      if ~isempty(soln)
        plotSlatSolution(prob, handles.model, soln, step, 'fr');
      else
        cla;
        axis off;
        h=text(0.5, 0.5, 'The solution has not been computed yet.');
        set(h,'horizontalalignment', 'center');
        set(h, 'verticalalignment', 'middle');
      end
      % Need to add tags for deciding which step to use
    otherwise
                  % Graph view
  end
end

function str=time2str(time)
str=sprintf('%d:%02d', floor(time/60), floor(mod(time,60)));

function timerFunction(object, event, hObject)
handles = guidata(hObject);
disp(['TimerFunc: ' num2str(handles.step)])
dt = now-handles.prevTime;
handles.prevTime = now;
handles.step=handles.step+dt*handles.prob.fps*get(handles.SpeedSlider, 'value')*3600*24;
if handles.step>handles.prob.nSteps
  handles.step = handles.prob.nSteps;
  stop(object);
  handles.playing = 0;
end
guidata(hObject, handles);
%figure(hObject); % hObject is the parent figure
updateView(handles, 1);
drawnow;
pause(0.05);
guidata(hObject, handles);

% --- Executes just before nips_demo is made visible.
function nips_demo_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to nips_demo (see VARARGIN)

% Choose default command line output for nips_demo
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes nips_demo wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = nips_demo_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in radiobutton1.
function radiobutton1_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton1


% --- Executes on button press in radiobutton2.
function radiobutton2_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton2


% --------------------------------------------------------------------
function FileMenu_Callback(hObject, eventdata, handles)
% hObject    handle to FileMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function DataMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to DataMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
guidata(hObject, setView(handles, 1));

% --------------------------------------------------------------------
function ViewMenu_Callback(hObject, eventdata, handles)
% hObject    handle to ViewMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function OpenMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to OpenMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if isfield(handles,'filename')
  dir = fileparts(handles.filename);
else
  dir = '~/pokusy/slat/intel50'
end
[file, path] = uigetfile('*.mat', 'Select a scenario to load', [dir '/']);

if ~isequal(file, 0)
  filename = [path file];
  a=load(filename);
  handles.filename=filename;
  handles.prob=a.prob;
  handles.model=a.model;
  %handles.model.R = 1600;  %!!! if only i could change the parameters on the fly...
  if isfield(a, 'kf')
    handles.kf=a.kf;
  else
    handles.kf=[];
  end
  if isfield(a, 'dbk')
    handles.dbk=a.dbk;
  else
    handles.dbk=[];
  end
  [path, name] = fileparts(filename);
  handles.datapath=[path '/' name];
  handles=resetView(handles);
  guidata(hObject, handles);
end

% --------------------------------------------------------------------
function SaveMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to SaveMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
prob = handles.prob;
model=handles.model;
kf=handles.kf;
dbk=handles.dbk;
save(handles.filename, 'prob', 'model', 'kf', 'dbk');

% --------------------------------------------------------------------
function ResultsMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to ResultsMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
guidata(hObject, setView(handles, 2));

% --------------------------------------------------------------------
function GraphsMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to GraphsMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
guidata(hObject, setView(handles, 3));

% --------------------------------------------------------------------
function OptionsMenu_Callback(hObject, eventdata, handles)
% hObject    handle to OptionsMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function CaptureOptions_Callback(hObject, eventdata, handles)
% hObject    handle to CaptureOptions (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function DistributedFilteringOptions_Callback(hObject, eventdata, handles)
% hObject    handle to DistributedFilteringOptions (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function ModelOptions_Callback(hObject, eventdata, handles)
% hObject    handle to ModelOptions (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on slider movement.
function TimeSlider_Callback(hObject, eventdata, handles)
% hObject    handle to TimeSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.step = get(hObject, 'value');
updateView(handles);
guidata(hObject, handles);


% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider


% --- Executes during object creation, after setting all properties.
function TimeSlider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to TimeSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end


% --- Executes on button press in pushbutton2.
function pushbutton2_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in pushbutton3.
function pushbutton3_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in pushbutton6.
function pushbutton6_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in NextStepButton.
function NextStepButton_Callback(hObject, eventdata, handles)
% hObject    handle to NextStepButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if isfield(handles, 'prob')
  handles.step = min(handles.step+1,handles.prob.nSteps);
  guidata(hObject, handles);
  updateView(handles, 1);
end


% --- Executes on button press in pushbutton8.
function pushbutton8_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton8 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on slider movement.
function SpeedSlider_Callback(hObject, eventdata, handles)
% hObject    handle to SpeedSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider


% --- Executes during object creation, after setting all properties.
function SpeedSlider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to SpeedSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end


% --- Executes during object creation, after setting all properties.
function figure1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called
handles.view = 1; % undefined
%setView(handles, 1); % Capture and visualization
handles.playing = 0;
handles.step = 1;
%handles.timer = timer('TimerFcn','disp(''Hello World!'')', 'BusyMode', 'drop', 'ExecutionMode', 'fixedRate');
handles.timer = timer('TimerFcn', {@timerFunction, hObject}, 'BusyMode', 'drop', 'ExecutionMode', 'fixedRate', 'Period', 0.1  );
%handles.runningDBK = 0;
%handles.dbktimer = timer('TimerFcn', {@dbkTimerFunction, hObject}, 'BusyMode', 'drop', 'ExecutionMode', 'fixedRate', 'Period', 1);
guidata(hObject, handles);


% --- If Enable == 'on', executes on mouse press in 5 pixel border.
% --- Otherwise, executes on mouse press in 5 pixel border or over pushbutton3.
function pushbutton3_ButtonDownFcn(hObject, eventdata, handles)
% hObject    handle to pushbutton3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in PrevStepButton.
function PrevStepButton_Callback(hObject, eventdata, handles)
% hObject    handle to PrevStepButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.step = max(handles.step-1,1);
guidata(hObject, handles);
updateView(handles, 1);


% --- Executes on button press in BeginButton.
function BeginButton_Callback(hObject, eventdata, handles)
% hObject    handle to BeginButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.step = 1;
guidata(hObject, handles);
updateView(handles, 1);


% --- Executes on button press in EndButton.
function EndButton_Callback(hObject, eventdata, handles)
% hObject    handle to EndButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if isfield(handles, 'prob')
  handles.step = handles.prob.nSteps;
  guidata(hObject, handles);
  updateView(handles, 1);
end


% --- Executes on button press in PlayStopButton.
function PlayStopButton_Callback(hObject, eventdata, handles)
% hObject    handle to PlayStopButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.playing
handles.playing = ~handles.playing;
handles.prevTime = now;
if handles.playing
  if handles.step >= handles.prob.nSteps
    handles.step = 1;
  end
  guidata(hObject, handles);
  start(handles.timer);
else
  stop(handles.timer);
  guidata(hObject, handles);
  updateView(handles);
end


% --- Executes on button press in CaptureButton.
function CaptureButton_Callback(hObject, eventdata, handles)
% hObject    handle to CaptureButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
[prob, datapath] = collect_data;
if ~isempty(prob),
  model = generateSlatModel(prob, 3, 0, 2, 0, 1);
  handles.prob     = prob;
  handles.model    = model;
  handles.filename = datapath;
  handles.datapath = datapath;
  handles.kf       = [];
  handles.dbk      = [];
  save(handles.filename, 'prob', 'model');
  %saveSlatProblem(handles.prob, [handles.datapath '/prob.lisp']);
  subplot(1,1,1);
  handles=resetView(handles);
  guidata(hObject, handles);
end


% --- Executes during object creation, after setting all properties.
function DataPanel_CreateFcn(hObject, eventdata, handles)
% hObject    handle to DataPanel (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called


% --- Executes on button press in RunKFButton.
function RunKFButton_Callback(hObject, eventdata, handles)
% hObject    handle to RunKFButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
kf = slatKFuv(handles.prob, handles.model);
kf.transform=slatComputeLRTransform(handles.prob, handles.model, kf);
handles.kf=kf;
updateView(handles);
guidata(hObject, handles);


% --- Executes on button press in RunDBKButton.
function RunDBKButton_Callback(hObject, eventdata, handles)
% hObject    handle to RunDBKButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%  msgbox('Distributed BK is already running', 'Error');
%else
  %handles.runningDBK = 1;
probfile = [handles.datapath '/prob.lisp'];
saveSlatProblem(handles.prob, probfile);
cmd_str = ['!xterm -e ./rundbk ' handles.datapath '&'];
disp(['Executing: ' cmd_str]);
eval(cmd_str);
guidata(hObject, handles);

% --- Executes on button press in ViewKFButton.
function ViewKFButton_Callback(hObject, eventdata, handles)
% hObject    handle to ViewKFButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
updateView(handles);

% --- Executes on button press in ViewDBKButton.
function ViewDBKButton_Callback(hObject, eventdata, handles)
% hObject    handle to ViewDBKButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
updateView(handles);


% --- Executes on button press in RunKFButton.
function pushbutton15_Callback(hObject, eventdata, handles)
% hObject    handle to RunKFButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in CaptureButton.
function pushbutton16_Callback(hObject, eventdata, handles)
% hObject    handle to CaptureButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in RunDBKButton.
function pushbutton18_Callback(hObject, eventdata, handles)
% hObject    handle to RunDBKButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in checkbox1.
function checkbox1_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox1


% --- Executes on button press in checkbox2.
function checkbox2_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox2


% --- Executes on button press in LoadDBKButton.
function LoadDBKButton_Callback(hObject, eventdata, handles)
% hObject    handle to LoadDBKButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
dbk = loadSlatResults(handles.datapath, 'dbk');
%dbk.transform=slatComputeOptimalTransform(handles.prob, handles.model, dbk);
handles.dbk=dbk;
guidata(hObject, handles);