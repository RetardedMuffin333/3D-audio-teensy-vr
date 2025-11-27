% export_hrir_to_teensy.m
% Load HRIR data and export int16 FIR filters for Teensy
%
% User: put your HRIR .mat file here:

clear; clc;
hrirFile = 'IRC_1038_C_HRIR.mat';   % or your own .mat

load(hrirFile);  % must define l_eq_hrir_S, r_eq_hrir_S or similar

numTaps = 128;           % number of FIR taps used on Teensy
targetElev = 0;          % example: horizontal plane only

% --- SELECT CHANNELS / POSITIONS ------------------------------------
% Find indices on desired elevation
idxPlane = find(l_eq_hrir_S.elev_v == targetElev);
aziVals = l_eq_hrir_S.azim_v(idxPlane);
[aziVals, sortIdx] = sort(aziVals);
idxPlane = idxPlane(sortIdx);

numPos = numel(idxPlane);
fprintf('Using %d positions at elevation %d deg.\n', numPos, targetElev);

L = zeros(numPos, numTaps, 'int16');
R = zeros(numPos, numTaps, 'int16');

for i = 1:numPos
    ch = idxPlane(i);

    hL = l_eq_hrir_S.content_m(ch, 1:numTaps);
    hR = r_eq_hrir_S.content_m(ch, 1:numTaps);

    % Q15 fixed-point
    qL = int16(max(min(round(hL * 32767), 32767), -32768));
    qR = int16(max(min(round(hR * 32767), 32767), -32768));

    % flip for convolution if needed
    qL = flip(qL);
    qR = flip(qR);

    L(i,:) = qL;
    R(i,:) = qR;
end

% --- WRITE C HEADER --------------------------------------------------
outFile = fullfile(fileparts(fileparts(mfilename('fullpath'))),'firmware','hrtf_filters.h');
fid = fopen(outFile, 'w');
if fid == -1
    error('Failed to open file: %s', outFile);
end

fprintf(fid, '#ifndef HRTF_FILTERS_H\n#define HRTF_FILTERS_H\n\n');
fprintf(fid, '// Generated from %s by export_hrir_to_teensy.m\n', hrirFile);
fprintf(fid, '#define NUM_TAPS %d\n', numTaps);
fprintf(fid, '#define NUM_POSITIONS %d\n\n', numPos);

% Optional: azimuth list for info
fprintf(fid, 'const float hrtfAzimuth[NUM_POSITIONS] = {\n');
for i = 1:numPos
    fprintf(fid, '  %.1ff', aziVals(i));
    if i < numPos, fprintf(fid, ','); end
    fprintf(fid, '\n');
end
fprintf(fid, '};\n\n');

% Left ear filters
fprintf(fid, 'const int16_t hrtfL[NUM_POSITIONS][NUM_TAPS] = {\n');
for i = 1:numPos
    fprintf(fid, '  {');
    for n = 1:numTaps
        fprintf(fid, '%d', L(i,n));
        if n < numTaps
            fprintf(fid, ',');
        end
    end
    if i < numPos
        fprintf(fid, '},\n');
    else
        fprintf(fid, '}\n');
    end
end
fprintf(fid, '};\n\n');


% Right ear filters
fprintf(fid, 'const int16_t hrtfR[NUM_POSITIONS][NUM_TAPS] = {\n');
for i = 1:numPos
    fprintf(fid, '  {');
    for n = 1:numTaps
        fprintf(fid, '%d', R(i,n));
        if n < numTaps
            fprintf(fid, ',');
        end
    end
    if i < numPos
        fprintf(fid, '},\n');
    else
        fprintf(fid, '}\n');   % <--- no comma for the last row
    end
end
fprintf(fid, '};\n\n');


fprintf(fid, '#endif // HRTF_FILTERS_H\n');
fclose(fid);

fprintf('Wrote %s\n', outFile);
