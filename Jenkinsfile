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

			docker.image("${dockerImage}").inside("") {

				checkout(scm);

				if (env.CHANGE_ID) {
					echo("Trying to build pull request");
				}

				if (!env.CHANGE_ID) {

				}

				sh("mkdir -p build/");
				sh("mkdir -p lib/");
				sh("rm -rfv build/*");
				sh("rm -rfv lib/*");

				discordSend(description: "", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Starting ${os} build target...", webhookURL: env.GS2EMU_WEBHOOK);

				dir("build") {
					sh("cmake -G\"${generator}\" -DCMAKE_BUILD_TYPE=Release ${defines} -DVER_EXTRA=\"-${fixed_os}-${fixed_job_name}\" .. || true"); // Temporary fix for Windows MingW builds
					sh("cmake --build . --config Release --target all -- -j `nproc`");
				}

				archiveArtifacts(artifacts: 'lib/*.dylib,lib/*.so,bin/*.dll', allowEmptyArchive: true);
				stash(name: osdir, includes: 'lib/*.dylib,lib/*.so,bin/*.dll', allowEmpty: true);

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

	def customImage = docker.image("mcr.microsoft.com/dotnet/sdk:8.0");
	customImage.pull();

	try {
		checkout scm;

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
			def release_type = ("${release_name}").replace('/','-').replace('GS2Engine-','').replace('main','').replace('dev','');

			stage("Building NuGet Package") {

				customImage.inside("-u 0") {
					sh("chmod 777 -R .");
					sh("dotnet pack GS2Compiler.csproj -c Release ${VER}");
					sh("chmod 777 -R .");
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

	env.COMMIT_MSG = sh (
		script: 'git log -1 --pretty=%B ${GIT_COMMIT}',
		returnStdout: true
	).trim();

	discordSend description: "${env.COMMIT_MSG}", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Build Started: ${fixed_job_name} #${env.BUILD_NUMBER}", webhookURL: env.GS2EMU_WEBHOOK



	def branches = [:];
	def project = readJSON file: "JenkinsEnv.json";

	project.builds.each { v ->
		branches["Build ${v.DockerRoot}/${v.DockerImage}:${v.DockerTag}"] = {
			node {
				buildStep(v.DockerImage, v.Generator, v.OS, v.OSDir, v.Defines);
			}
		}
	}

	sh("rm -rf ./*");

	parallel(branches);

	def customImage = docker.image("mcr.microsoft.com/dotnet/sdk:8.0");
	customImage.pull();

	project.builds.each { v ->
        sh("mkdir -p bindings/dotnet/cross-compile/${v.OSDir}/");

        dir("bindings/dotnet/cross-compile/${v.OSDir}/") {
            unstash(name: v.OSDir);
        }
    }

    dir("bindings/dotnet/") {
        buildStepDocker();
    }
}