function varargout = collect_data(varargin)
% COLLECT_DATA M-file for collect_data.fig
%      COLLECT_DATA by itself, creates a new COLLECT_DATA or raises the
%      existing singleton*.
%
%      H = COLLECT_DATA returns the handle to a new COLLECT_DATA or the handle to
%      the existing singleton*.
%
%      COLLECT_DATA('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in COLLECT_DATA.M with the given input arguments.
%
%      COLLECT_DATA('Property','Value',...) creates a new COLLECT_DATA or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before collect_data_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to collect_data_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help collect_data

% Last Modified by GUIDE v2.5 03-Dec-2005 00:12:12

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @collect_data_OpeningFcn, ...
                   'gui_OutputFcn',  @collect_data_OutputFcn, ...
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

% --- Executes just before collect_data is made visible.
function collect_data_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to collect_data (see VARARGIN)

% Choose default command line output for collect_data
handles.prob= []; 
handles.datapath=[];

% Update handles structure
guidata(hObject, handles);

% Insert custom Title and Text if specified by the user
% Hint: when choosing keywords, be sure they are not easily confused 
% with existing figure properties.  See the output of set(figure) for
% a list of figure properties.
if(nargin > 3)
    for index = 1:2:(nargin-3),
        if nargin-3==index, break, end
        switch lower(varargin{index})
         case 'title'
          set(hObject, 'Name', varargin{index+1});
         case 'string'
          set(handles.text1, 'String', varargin{index+1});
        end
    end
end

% Determine the position of the dialog - centered on the callback figure
% if available, else, centered on the screen
FigPos=get(0,'DefaultFigurePosition');
OldUnits = get(hObject, 'Units');
set(hObject, 'Units', 'pixels');
OldPos = get(hObject,'Position');
FigWidth = OldPos(3);
FigHeight = OldPos(4);
if isempty(gcbf)
    ScreenUnits=get(0,'Units');
    set(0,'Units','pixels');
    ScreenSize=get(0,'ScreenSize');
    set(0,'Units',ScreenUnits);

    FigPos(1)=1/2*(ScreenSize(3)-FigWidth);
    FigPos(2)=2/3*(ScreenSize(4)-FigHeight);
else
    GCBFOldUnits = get(gcbf,'Units');
    set(gcbf,'Units','pixels');
    GCBFPos = get(gcbf,'Position');
    set(gcbf,'Units',GCBFOldUnits);
    FigPos(1:2) = [(GCBFPos(1) + GCBFPos(3) / 2) - FigWidth / 2, ...
                   (GCBFPos(2) + GCBFPos(4) / 2) - FigHeight / 2];
end
FigPos(3:4)=[FigWidth FigHeight];
set(hObject, 'Position', FigPos);
set(hObject, 'Units', OldUnits);

% Show a question icon from dialogicons.mat - variables questIconData
% and questIconMap
load dialogicons.mat

%IconData=questIconData;
%questIconMap(256,:) = get(handles.figure1, 'Color');
%IconCMap=questIconMap;

%Img=image(IconData, 'Parent', handles.axes1);
%set(handles.figure1, 'Colormap', IconCMap);

%set(handles.axes1, ...
%    'Visible', 'off', ...
%    'YDir'   , 'reverse'       , ...
%    'XLim'   , get(Img,'XData'), ...
%    'YLim'   , get(Img,'YData')  ...
%    );

% Make the GUI modal
set(handles.figure1,'WindowStyle','modal')

% UIWAIT makes collect_data wait for user response (see UIRESUME)
uiwait(handles.figure1);

% --- Outputs from this function are returned to the command line.
function varargout = collect_data_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.prob;
varargout{2} = handles.datapath;

% The figure can be deleted now
delete(handles.figure1);

% --- Executes on button press in StartButton.
function StartButton_Callback(hObject, eventdata, handles)
% hObject    handle to StartButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

name = get(handles.Name, 'String');
if length(name)>0,
  delay = str2num(get(handles.StartTime,'String'));
  start = round(datevec(now+delay/86400));
  duration = str2num(get(handles.Duration, 'String'));
  
  start_str    = sprintf('%04d.%02d.%02d.%02d%02d%02d', start);
  duration_str = num2str(duration);
  
  cmd_str = ['!cd ' handles.testbed_dir '; ./collection_sequence ' ...
    handles.experiment_root ' ' name ' ' start_str ' ' duration_str];
  disp(['Executing: ' cmd_str]);
  eval(cmd_str);
  prob = generateSlatIntel(0,0.05);
  prob = loadSlatObservations(prob, [handles.experiment_root '/' name]);
  handles.prob = prob;
  handles.datapath=[handles.experiment_root '/' name];

  %save(handles.filename, 'prob');
  
  % Update handles structure
  guidata(hObject, handles);
  
  % Use UIRESUME instead of delete because the OutputFcn needs
  % to get the updated handles structure.
  uiresume(handles.figure1);
end

% --- Executes on button press in CancelButton.
function CancelButton_Callback(hObject, eventdata, handles)
% hObject    handle to CancelButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Update handles structure
guidata(hObject, handles);

% Use UIRESUME instead of delete because the OutputFcn needs
% to get the updated handles structure.
uiresume(handles.figure1);


% --- Executes when user attempts to close figure1.
function figure1_CloseRequestFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if isequal(get(handles.figure1, 'waitstatus'), 'waiting')
    % The GUI is still in UIWAIT, use UIRESUME
    uiresume(handles.figure1);
else
    % The GUI is no longer waiting, just close it
    delete(handles.figure1);
end


% --- Executes on key press over figure1 with no controls selected.
function figure1_KeyPressFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Check for "enter" or "escape"
if isequal(get(hObject,'CurrentKey'),'escape')
    % User cancelled => use the defaults (no prob)
    uiresume(handles.figure1);
end    
    
%if isequal(get(hObject,'CurrentKey'),'return')
%    uiresume(handles.figure1);
%end    



function StartTime_Callback(hObject, eventdata, handles)
% hObject    handle to StartTime (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of StartTime as text
%        str2double(get(hObject,'String')) returns contents of StartTime as a double


% --- Executes during object creation, after setting all properties.
function StartTime_CreateFcn(hObject, eventdata, handles)
% hObject    handle to StartTime (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Duration_Callback(hObject, eventdata, handles)
% hObject    handle to Duration (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Duration as text
%        str2double(get(hObject,'String')) returns contents of Duration as a double


% --- Executes during object creation, after setting all properties.
function Duration_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Duration (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Name_Callback(hObject, eventdata, handles)
% hObject    handle to Name (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Name as text
%        str2double(get(hObject,'String')) returns contents of Name as a double


% --- Executes during object creation, after setting all properties.
function Name_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Name (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in Progress1.
function Progress1_Callback(hObject, eventdata, handles)
% hObject    handle to Progress1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of Progress1


% --- Executes on button press in Progress2.
function Progress2_Callback(hObject, eventdata, handles)
% hObject    handle to Progress2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of Progress2


% --- Executes on button press in checkbox3.
function checkbox3_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox3


% --- Executes on button press in checkbox4.
function checkbox4_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox4


% --- Executes during object creation, after setting all properties.
function figure1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called


handles.experiment_root = '~/pokusy/slat/intel50';
handles.testbed_dir = '~/repo/testbeds/cameras/intel50';

% Update handles structure
guidata(hObject, handles);
