% LOAD_OPTIONS - Loads options from a configuration file
%                This function provides a simple means of
%                parsing attribute-value options.  Each option is
%                named by a unique string and is given a default
%                value.
%
% Usage:  [var1, var2, ..., varn[, unused]] = ...
%           process_options(filename, ...
%                           str1, def1, str2, def2, ..., strn, defn)
%
% Arguments:   
%            args            - a cell array of input arguments, such
%                              as that provided by VARARGIN.  Its contents
%                              should alternate between strings and
%                              values.
%            str1, ..., strn - Strings that are associated with a 
%                              particular variable
%            def1, ..., defn - Default values returned if no option
%                              is supplied.  Each default value also
%                              determines the expected type of the
%                              parameter.
%
% Returns:
%            var1, ..., varn - values to be assigned to variables
%            unused          - an optional cell array of those 
%                              string-value pairs that were unused;
%                              if this is not supplied, then a
%                              warning will be issued for each
%                              option in args that lacked a match.
%
% Examples (these need to be updated from Mark's process-Options):
%
% Suppose we wish to define a Matlab function 'func' that has
% required parameters x and y, and optional arguments 'u' and 'v'.
% With the definition
%
%   function y = func(x, y, varargin)
%
%     [u, v] = process_options(varargin, 'u', 0, 'v', 1);
%
% calling func(0, 1, 'v', 2) will assign 0 to x, 1 to y, 0 to u, and 2
% to v.  The parameter names are insensitive to case; calling 
% func(0, 1, 'V', 2) has the same effect.  The function call
% 
%   func(0, 1, 'u', 5, 'z', 2);
%
% will result in u having the value 5 and v having value 1, but
% will issue a warning that the 'z' option has not been used.  On
% the other hand, if func is defined as
%
%   function y = func(x, y, varargin)
%
%     [u, v, unused_args] = process_options(varargin, 'u', 0, 'v', 1);
%
% then the call func(0, 1, 'u', 5, 'z', 2) will yield no warning,
% and unused_args will have the value {'z', 2}.  This behaviour is
% useful for functions with options that invoke other functions
% with options; all options can be passed to the outer function and
% its unprocessed arguments can be passed to the inner function.

% Copyright (C) 2002, 2005 Mark Paskin, Stanislav Funiak
%
% This program is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation; either version 2 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful, but
% WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
% USA.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [varargout] = load_options(filename, varargin)

% Todo: Ammend the interface to make certain parameters required.

% Check the number of input arguments
n = length(varargin);
if (mod(n, 2))
  error('Each option must be a string/value pair.');
end

% Check the number of supplied output arguments
if (nargout < (n / 2))
  error('Insufficient number of output arguments given');
elseif (nargout == (n / 2))
  warn = 1;
  nout = n / 2;
else
  warn = 0;
  nout = n / 2 + 1;
end

% Set outputs to be defaults
varargout = cell(1, nout);
for i=2:2:n
  varargout{i/2} = varargin{i};
end


% Now process all arguments
nunused = 0;
fid = fopen(filename, 'r');
line = fgetl(fid);
lineno = 0;

while ~any(line==-1) 
  strings = textscan(line, '%s%q%s', 1, 'commentStyle', '#', ...
    'whitespace', '= \b\t');
  line = fgetl(fid);
  lineno = lineno+1;
  
  if length(strings{1})>0  % Something to parse
    if length(strings{2})==0
      error(['Line ' num2str(lineno) ' of ' filename ': missing value.']);
    end
    if length(strings{3})>0
      warning(['Line ' num2str(lineno) ' of ' filename ': extra values, ignoring']);
    end
    
    argname = strings{1}{1};
    argvalue = strings{2}{1};
    found = 0;
    for j=1:2:n
      if strcmpi(argname, varargin{j})

        % Convert to the desired value type
        if ischar(varargin{j+1})
          % The output is already a string
        elseif islogical(varargin{j+1})
          switch lower(argvalue)
            case {'yes', 'true', '1'}
              argvalue=true;
            case {'no', 'false', '0'}
              argvalue=false;
            otherwise
              error(['Line ' num2str(lineno) ' of ' filename ': invalid logical value ' argvalue]);
          end
        elseif isnumeric(varargin{j+1})
          argvalue = str2num(argvalue);
          if ~all(size(argvalue)==size(varargin{j+1}))
            error(['Line ' num2str(lineno) ' of ' filename ': invalid number or array size']);
          end
        else
          error(['Invalid default value type for argument ' argname]);
        end
        varargout{(j + 1)/2} = argvalue;
        found = 1;
        break;
      end
    end
    if (~found)
      if (warn)
        warning(sprintf('Option ''%s'' not used.', argname));
      else
        nunused = nunused + 1;
        unused{2 * nunused - 1} = argname;
        unused{2 * nunused} = argvalue;
      end
    end
  end
end

fclose(fid);

% Assign the unused arguments
if (~warn)
  if (nunused)
    varargout{nout} = unused;
  else
    varargout{nout} = cell(0);
  end
end