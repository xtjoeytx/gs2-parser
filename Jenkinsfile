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

def buildStep(dockerImage, generator, os, defines) {
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

				discordSend description: "", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Starting ${os} build target...", webhookURL: env.GS2EMU_WEBHOOK

				dir("build") {
					sh("cmake -G\"${generator}\" -DCMAKE_BUILD_TYPE=Release ${defines} -DVER_EXTRA=\"-${fixed_os}-${fixed_job_name}\" .. || true"); // Temporary fix for Windows MingW builds
					sh("cmake --build . --config Release --target all -- -j `nproc`");

					archiveArtifacts(artifacts: '*.zip,*.tar.gz,*.tgz');
				}

				discordSend description: "", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Build ${fixed_job_name} #${env.BUILD_NUMBER} Target: ${os} successful!", webhookURL: env.GS2EMU_WEBHOOK
			}
		}
	} catch(err) {
		discordSend description: "", footer: "", link: env.BUILD_URL, result: currentBuild.currentResult, title: "[${split_job_name[0]}] Build Failed: ${fixed_job_name} #${env.BUILD_NUMBER}", webhookURL: env.GS2EMU_WEBHOOK
		currentBuild.result = 'FAILURE'
		notify('Build failed')
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
				buildStep(v.DockerImage, v.Generator, v.OS, v.Defines);
			}
		}
	}

	sh("rm -rf ./*");

	parallel(branches);
}