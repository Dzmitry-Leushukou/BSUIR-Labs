// wwwroot/js/site.js
$(function () {
    const $container = $("#carContainer");

    function ajaxLoad(url, onDone) {
        if (!url) return;
        $container.addClass("is-loading");
        $container.load(url, function (response, status, xhr) {
            $container.removeClass("is-loading");

            if (status === "error") {
                console.error("Ajax load failed:", xhr.status, xhr.statusText);
                window.location = url;
                return;
            }

            if (typeof onDone === "function") onDone();

            const top = $container.offset().top;
            window.scrollTo({ top, behavior: "smooth" });
        });
    }

    $(document).on("click", "#carContainer .pagination .page-link", function (e) {
        if (e.ctrlKey || e.metaKey || e.shiftKey || e.which === 2) return;

        const $li = $(this).closest(".page-item");
        if ($li.hasClass("disabled") || $li.hasClass("active")) {
            e.preventDefault();
            return;
        }

        e.preventDefault();
        ajaxLoad($(this).attr("href"));
    });

    $(document).on("click", ".dropdown-menu .dropdown-item", function (e) {
        if (e.ctrlKey || e.metaKey || e.shiftKey || e.which === 2) return;
        e.preventDefault();
        const url = $(this).attr("href");
        const text = $(this).text().trim();
        ajaxLoad(url, function () {
            $("#dropdown").text(text);
        });
    });
});
