/* Theme changer */

function pageLoad() {

    console.log("Loaded");
    document.getElementById("TextArea1").value = localStorage.getItem("stickyText");

    $(document).ready(function () {
        $(".dashboard").draggable()
        $("#insidediv").draggable()
        $("#themelist").bind("change", function () {
            switch ($("#themelist").val()) {

                case "Blue":
                    $(".dashboard").removeClass("dashboard").addClass("dashboardblue");
                    $(".dashboardazure").removeClass("dashboardazure").addClass("dashboardblue");
                    break;

                case "Azure":
                    $(".dashboardblue").removeClass("dashboardblue").addClass("dashboardazure");
                    $(".dashboard").removeClass("dashboard").addClass("dashboardazure");
                    break;

                case "Pink":
                    break;

                case "Coral":
                    $(".dashboardblue").removeClass("dashboardblue").addClass("dashboard");
                    $(".dashboardazure").removeClass("dashboardazure").addClass("dashboard");
                    break;
            }
        });
    });
}

function pageunload() {
    console.log("Sticky: " + document.getElementById("TextArea1").value);
    localStorage.setItem("stickyText", document.getElementById("TextArea1").value);
}