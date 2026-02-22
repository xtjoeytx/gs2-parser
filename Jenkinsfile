def notify(status){
	emailext (
		body: '$DEFAULT_CONTENT',
		recipientProviders: [
			[$class: 'CulpritsRecipientProvider'],
			[$class: 'DevelopersRecipientProvider'],
			[$class: 'RequesterRecipientProvider']
		],
		replyTo: '$DEFAULT_REPLYTO',
		subject: '$DEFAULT_SUBJECT',
		to: '$DEFAULT_RECIPIENTS'
	)
}

@NonCPS
def killall_jobs() {
	def jobname = env.JOB_NAME;
	def buildnum = env.BUILD_NUMBER.toInteger();
	def killnums = "";
	def job = Jenkins.instance.getItemByFullName(jobname);
	def split_job_name = env.JOB_NAME.split(/\/{1}/);
	def fixed_job_name = split_job_name[1].replace('%2F',' ');

	for (build in job.builds) {
		if (!build.isBuilding()) { continue; }
		if (buildnum == build.getNumber().toInteger()) { continue; println "equals"; }
		if (buildnum < build.getNumber().toInteger()) { continue; println "newer"; }

		echo("Kill task = ${build}");

		killnums += "#" + build.getNumber().toInteger() + ", ";

		build.doStop();
	}

	if (killnums != "") {
		discordSend description: "in favor of #${buildnum}, ignore following failed builds for ${killnums}", footer: "", link: env.BUILD_URL, result: "ABORTED", title: "[${split_job_name[0]}] Killing task(s) ${fixed_job_name} ${killnums}", webhookURL: env.GS2EMU_WEBHOOK
	}
	echo("Done killing");
}

def buildStep(dockerImage, generator, os, osdir, defines) {
	def split_job_name = env.JOB_NAME.split(/\/{1}/);
	def fixed_job_name = split_job_name[1].replace('%2F','-');
	fixed_job_name = fixed_job_name.replace('/','-');
    def fixed_os = os.replace(' ','-');

	try{
		stage("Building on \"${dockerImage}\" with \"${generator}\" for \"${os}\"...") {
			properties([pipelineTriggers([githubPush()])]);
			def commondir = env.WORKSPACE + '/../' + fixed_job_name + '/';

			def dockerImageRef = docker.image("${dockerImage}");
			dockerImageRef.pull();

			dockerImageRef.inside("") {

				checkout(scm);

				if (env.CHANGE_ID) {
					echo("Trying to build pull request");
				}

				if (!env.CHANGE_ID) {

				}

				sh("mkdir -p build/");
				sh("rm -rfv build/*");

				discordSend(description: "", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Starting ${os} build target...", webhookURL: env.GS2EMU_WEBHOOK);

				dir("build") {
					sh("cmake -G\"${generator}\" -DCMAKE_BUILD_TYPE=Release ${defines} -DVER_EXTRA=\"-${fixed_os}-${fixed_job_name}\" .. || true"); // Temporary fix for Windows MingW builds
					sh("cmake --build . --config Release --target all -- -j `nproc`");
				}

				archiveArtifacts(artifacts: 'build/lib/*.dylib,build/lib/*.so,build/bin/*.dll', allowEmptyArchive: true);
				stash(name: osdir, includes: 'build/lib/*.dylib,build/lib/*.so,build/bin/*.dll', allowEmpty: true);

				discordSend(description: "", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Build ${fixed_job_name} #${env.BUILD_NUMBER} Target: ${os} successful!", webhookURL: env.GS2EMU_WEBHOOK);
			}
		}
	} catch(err) {
		discordSend(description: "", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Build Failed: ${fixed_job_name} #${env.BUILD_NUMBER}", webhookURL: env.GS2EMU_WEBHOOK);
		currentBuild.result = 'FAILURE';
		notify('Build failed');
		throw err;
	}
}

def buildStepDocker() {
	def split_job_name = env.JOB_NAME.split(/\/{1}/);
	def fixed_job_name = split_job_name[1].replace('%2F',' ');

	def customImage = docker.image("mcr.microsoft.com/dotnet/sdk:10.0");
	customImage.pull();

	try {
        def buildenv = "";
        def tag = '';
        def VER = '';
        def EXTRA_VER = '';


        if(env.TAG_NAME) {
            sh(returnStdout: true, script: "echo '```' > RELEASE_DESCRIPTION.txt");
            env.RELEASE_DESCRIPTION = sh(returnStdout: true, script: "git tag -l --format='%(contents)' ${env.TAG_NAME} >> RELEASE_DESCRIPTION.txt");
            sh(returnStdout: true, script: "echo '```' >> RELEASE_DESCRIPTION.txt");
        }

        if (env.BRANCH_NAME.equals('main')) {
            tag = "latest";
        } else {
            tag = "${env.BRANCH_NAME.replace('/','-')}";
        }

        if (env.TAG_NAME) {
            EXTRA_VER = "";
            VER = "/p:Version=${env.TAG_NAME}";
        } else if (env.BRANCH_NAME.equals('dev')) {
            EXTRA_VER = "-beta";
        } else {
            EXTRA_VER = "--build-arg VER_EXTRA=-${tag}";
        }

        docker.withRegistry("https://index.docker.io/v1/", "dockergraal") {
            def release_name = env.JOB_NAME.replace('%2F','/');
            def release_type = ("${release_name}").replace('/','-').replace('GS2Compiler-','').replace('main','').replace('dev','');

            stage("Building NuGet Package") {

                customImage.inside("-u 0") {
                    dir("bindings/dotnet/") {
                        sh("chmod 777 -R .");
                        sh("dotnet pack Preagonal.Scripting.GS2Compiler.csproj -c Release ${VER}");
                        sh("chmod 777 -R .");
                    }
                }
            }
            
            stage("Run tests...") {
				customImage.inside("-u 0") {
					dir("bindings/dotnet/") {
						try{
							sh("dotnet test -c Debug -r linux-x64 --logger \"trx;LogFileName=../../Testing/unit_tests.xml\"");
							sh("dotnet test -c Debug -r linux-x64 /p:CollectCoverage=true /p:CoverletOutputFormat=opencover");
							sh("chmod 777 -R .");
						} catch(err) {
							currentBuild.result = 'FAILURE'
							sh("chmod 777 -R .");
							discordSend(description: "Testing Failed: ${fixed_job_name} #${env.BUILD_NUMBER}", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Tests Failed: ${fixed_job_name} #${env.BUILD_NUMBER}", webhookURL: env.GS2EMU_WEBHOOK);
							notify('Build failed')
						}
	
						archiveArtifacts (
							artifacts: 'Testing/**.xml',
							fingerprint: true
						)
	
						withCredentials([string(credentialsId: 'PREAGONAL_GS2ENGINE_CODECOV_TOKEN', variable: 'CODECOV_TOKEN')]) {
							sh("curl -s https://codecov.io/bash > codecov && chmod +x codecov && ./codecov -f \"Testing/unit_tests.xml\" -t ${env.CODECOV_TOKEN} && ./codecov -f \"Preagonal.Scripting.GS2Compiler.UnitTests/coverage.opencover.xml\" -t ${env.CODECOV_TOKEN}")
						}
	
						stage("Xunit") {
							xunit (
								testTimeMargin: '3000',
								thresholdMode: 1,
								thresholds: [
									skipped(failureThreshold: '1000'),
									failed(failureThreshold: '0')
								],
								tools: [MSTest(
									pattern: 'Testing/**.xml',
									deleteOutputFiles: true,
									failIfNotNew: false,
									skipNoTestFiles: true,
									stopProcessingIfError: true
								)],
								skipPublishingChecks: false
							);
						}
					}
				}
			}

            def archive_date = sh (
                script: 'date +"-%Y%m%d-%H%M"',
                returnStdout: true
            ).trim();

            if (env.TAG_NAME) {
                archive_date = '';
            }

            if (env.TAG_NAME) {
                stage("Pushing NuGet") {
                    customImage.inside("-u 0") {
                        dir("bindings/dotnet/") {
							archiveArtifacts(artifacts: 'bin/Release/*.nupkg', allowEmptyArchive: true);
                            withCredentials([string(credentialsId: 'PREAGONAL_GITHUB_TOKEN', variable: 'GITHUB_TOKEN')]) {
                                sh("dotnet nuget push -s https://nuget.pkg.github.com/Preagonal/index.json -k ${env.GITHUB_TOKEN} bin/Release/*.nupkg;chmod 777 -R .");
                                discordSend description: "NuGet Successful", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Artifact Successful: ${fixed_job_name} #${env.BUILD_NUMBER}", webhookURL: env.GS2EMU_WEBHOOK;
                            }
                            withCredentials([string(credentialsId: 'PREAGONAL_NUGET_TOKEN', variable: 'NUGET_TOKEN')]) {
                                sh("dotnet nuget push -s https://api.nuget.org/v3/index.json -k ${env.NUGET_TOKEN} bin/Release/*.nupkg;chmod 777 -R .");
                                discordSend description: "NuGet Successful", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Artifact Successful: ${fixed_job_name} #${env.BUILD_NUMBER}", webhookURL: env.GS2EMU_WEBHOOK;
                            }
                        }
                    }
                }
            }
        }
	} catch(err) {
		currentBuild.result = 'FAILURE'
		customImage.inside("-u 0") {
			sh("chmod 777 -R .");
		}
		discordSend description: "", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Build Failed: ${fixed_job_name} #${env.BUILD_NUMBER}", webhookURL: env.GS2EMU_WEBHOOK

		notify("Build Failed: ${fixed_job_name} #${env.BUILD_NUMBER}")
		throw err
	}
}

node('master') {
killall_jobs();
	def split_job_name = env.JOB_NAME.split(/\/{1}/);
	def fixed_job_name = split_job_name[1].replace('%2F',' ');
	checkout(scm);

	env.COMMIT_MSG = sh(
		script: 'git log -1 --pretty=%B ${GIT_COMMIT}',
		returnStdout: true
	).trim();

	env.GIT_COMMIT = sh(
		script: 'git log -1 --pretty=%H ${GIT_COMMIT}',
		returnStdout: true
	).trim();

	sh('git fetch --tags');

	env.LATEST_TAG = sh(
		script: 'git tag --sort=creatordate -l | tail -1',
		returnStdout: true
	).trim();

	echo("Latest tag: ${env.LATEST_TAG}");

	def version = env.LATEST_TAG.split(/\./);

	echo("Version: ${version}");

	def verMajor = version[0] as Integer;
	def verMinor = version[1] as Integer;
	def verPatch = version[2] as Integer;
	def versionChanged = false;

	echo("Version - Major: ${verMajor}, Minor: ${verMinor}, Patch: ${verPatch}");

	if (env.BRANCH_NAME.equals('main')) {
		verMinor++;
		verPatch = 0;
		versionChanged = true;
	} else if (env.BRANCH_NAME.equals('dev')) {
		verPatch++;
		versionChanged = true;
	}

    if (versionChanged) {
        withCredentials([string(credentialsId: 'PREAGONAL_GITHUB_TOKEN', variable: 'GITHUB_TOKEN')]) {
            def tagName = "${verMajor}.${verMinor}.${verPatch}";

            def iso8601Date = sh(
                script: 'date -Iseconds',
                returnStdout: true
            ).trim();

            env.JSON_RESPONSE = sh(
                script: "curl -L -X POST -H \"Accept: application/vnd.github+json\" -H \"Authorization: Bearer ${env.GITHUB_TOKEN}\" -H \"X-GitHub-Api-Version: 2022-11-28\" https://api.github.com/repos/xtjoeytx/gs2-parser/git/tags -d '{\"tag\":\"${tagName}\",\"message\":\"${env.COMMIT_MSG}\",\"object\":\"${env.GIT_COMMIT}\",\"type\":\"tree\",\"tagger\":{\"name\":\"preagonal-pipeline[bot]\",\"email\":\"119898225+preagonal-pipeline[bot]@users.noreply.github.com\",\"date\":\"${iso8601Date}\"}}'",
                returnStdout: true
            );
            def response = readJSON(text: env.JSON_RESPONSE);

            sh(
                script: "curl -L -X POST -H \"Accept: application/vnd.github+json\" -H \"Authorization: Bearer ${env.GITHUB_TOKEN}\" -H \"X-GitHub-Api-Version: 2022-11-28\" https://api.github.com/repos/xtjoeytx/gs2-parser/git/refs -d '{\"ref\": \"refs/tags/${tagName}\", \"sha\": \"${response.sha}\"}'",
                returnStdout: true
            );
        }
    }

	discordSend description: "${env.COMMIT_MSG}", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Build Started: ${fixed_job_name} #${env.BUILD_NUMBER}", webhookURL: env.GS2EMU_WEBHOOK

	if (env.TAG_NAME) {
		sh(returnStdout: true, script: "echo '```' > RELEASE_DESCRIPTION.txt");
		env.RELEASE_DESCRIPTION = sh(returnStdout: true, script: "git tag -l --format='%(contents)' ${env.TAG_NAME} >> RELEASE_DESCRIPTION.txt");
		sh(returnStdout: true, script: "echo '```' >> RELEASE_DESCRIPTION.txt");
	}

	def branches = [:];
	def project = readJSON file: "JenkinsEnv.json";

	project.builds.each { v ->
		branches["Build ${v.DockerRoot}/${v.DockerImage}:${v.DockerTag}"] = {
			node("amd64") {
				buildStep(v.DockerImage, v.Generator, v.OS, v.OSDir, v.Defines);
			}
		}
	}

	parallel(branches);

	def customImage = docker.image("mcr.microsoft.com/dotnet/sdk:10.0");
	customImage.pull();

	project.builds.each { v ->
        sh("mkdir -p bindings/dotnet/cross-compile/${v.OSDir}/");

        dir("bindings/dotnet/cross-compile/${v.OSDir}/") {
            unstash(name: v.OSDir);
            try {
                sh("mv -fv build/bin/* .");
                sh("rm -rf build/bin")
            } catch(err) { }
            try {
                sh("mv -fv build/lib/* .");
                sh("rm -rf build/lib")
            } catch(err) { }
            try {
                sh("rm -rf build")
            } catch(err) { }
        }
    }

    dir("bindings/dotnet/") {
        sh("ls -l cross-compile/*");
    }

    buildStepDocker();
	if (env.TAG_NAME) {
		//def DESC = sh(returnStdout: true, script: 'cat RELEASE_DESCRIPTION.txt');
		//discordSend description: "${DESC}", customUsername: "OpenGraal", customAvatarUrl: "https://pbs.twimg.com/profile_images/1895028712/13460_106738052711614_100001262603030_51047_4149060_n_400x400.jpg", footer: "OpenGraal Team", link: "https://github.com/xtjoeytx/gs2-parser/pkgs/nuget/GS2Compiler", result: "SUCCESS", title: "GS2Compiler v${env.TAG_NAME} NuGet Package", webhookURL: env.GS2EMU_RELEASE_WEBHOOK;
	}

    sh("rm -rf ./*");
}