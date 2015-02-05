cppcheckflags = -I${top_srcdir} -I${top_srcdir}/src \
				--language=c \
				--enable=warning,performance,portability,unusedFunction,missingInclude,style \
				--suppressions-list=.cppcheck_suppressions                             \
				--library=sdl.cfg,posix.cfg                                                      \
				--inline-suppr                                                         \
				--inconclusive

cppcheckdefs = -DHAVE_ICONV -DWITH_RTPROF -D__GNUC__ -DHAVE_CONFIG_H -D_GNU_SOURCE=1 -D_REENTRANT

cppcheck:
	@echo "`cppcheck --version`"
	cppcheck $(cppcheckflags) $(cppcheckdefs) croppy gluem win32 src

cppcheck-report:
	@echo "`cppcheck --version`"
	mkdir -p cppcheck-report
	cppcheck $(cppcheckflags) $(cppcheckdefs) --xml-version=2 --xml  croppy gluem win32 src 2> cppcheck-report/cppcheck.xml
	cppcheck-htmlreport --file=cppcheck-report/cppcheck.xml --title="FreedroidRPG `git describe --tags 2>/dev/null || echo "@PACKAGE_VERSION@"`" --report-dir=cppcheck-report --source-dir=$(top_srcdir)

.PHONY: cppcheck cppcheck-report

gourceflags =	-c 0.8                           \
				--seconds-per-day 0.001          \
				--logo ./win32/w32icon_64x64.png \
				--auto-skip-seconds 0.0001       \
				--title "FreedroidRPG"           \
				--key                            \
				--camera-mode overview           \
				--highlight-all-users            \
				--file-idle-time 0               \
				--hide progress,mouse,filenames  \
				--stop-at-end                    \
				--max-files 99999999999          \
				--max-file-lag 0.000001          \
				--bloom-multiplier 1.3           \
				-1280x720

ffmpegflags =	-f image2pipe                    \
				-vcodec ppm                      \
				-i -                             \
				-y                               \
				-vcodec libx264                  \
				-preset medium                   \
				-crf 22                          \
				-pix_fmt yuv420p                 \
				-threads:0 2                     \
				-b:v 3000k                       \
				-maxrate 8000k                   \
				-bufsize 10000k FreedroidRPG.mp4


gource:
	gource $(gourceflags)

gource_ffmpeg:
	gource $(gourceflags) --output-ppm-stream - | ffmpeg $(ffmpegflags)

