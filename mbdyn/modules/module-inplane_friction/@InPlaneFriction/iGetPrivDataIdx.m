% MBDyn (C) is a multibody analysis code. 
% http://www.mbdyn.org
%
% Copyright (C) 1996-2015
%
% Pierangelo Masarati	<masarati@aero.polimi.it>
% Paolo Mantegazza	<mantegazza@aero.polimi.it>
%
% Dipartimento di Ingegneria Aerospaziale - Politecnico di Milano
% via La Masa, 34 - 20156 Milano, Italy
% http://www.aero.polimi.it
%
% Changing this copyright notice is forbidden.
%
% This program is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation (version 2 of the License).
% 
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

% AUTHOR: Reinhard Resch <r.resch@secop.com>
%        Copyright (C) 2011(-2015) all rights reserved.
%
%        The copyright of this code is transferred
%        to Pierangelo Masarati and Paolo Mantegazza
%        for use in the software MBDyn as described
%        in the GNU Public License version 2.1

function iPrivDataIdx = iGetPrivDataIdx(elem, name)
    switch (name)
    case "lambda"
        iPrivDataIdx = int32(1);
    case "z1"
        iPrivDataIdx = int32(2);
    case "z2"
        iPrivDataIdx = int32(3);
    case "zP1"
        iPrivDataIdx = int32(4);
    case "zP2"
        iPrivDataIdx = int32(5);
    otherwise
        error("inplane friction(%d): unknown private data name \"%s\"", elem.pMbElem.GetLabel(), name);
    endswitch
endfunction
