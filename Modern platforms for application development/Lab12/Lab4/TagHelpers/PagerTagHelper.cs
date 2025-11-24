// TagHelpers/PagerTagHelper.cs
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc.Rendering;
using Microsoft.AspNetCore.Razor.TagHelpers;
using Microsoft.AspNetCore.Routing;
using System;

namespace Lab1.Web.TagHelpers
{
    [HtmlTargetElement("Pager", TagStructure = TagStructure.NormalOrSelfClosing)]
    public class PagerTagHelper : TagHelper
    {
        private readonly LinkGenerator _linkGenerator;
        private readonly IHttpContextAccessor _httpContextAccessor;

        public PagerTagHelper(LinkGenerator linkGenerator, IHttpContextAccessor httpContextAccessor)
        {
            _linkGenerator = linkGenerator;
            _httpContextAccessor = httpContextAccessor;
        }

        [HtmlAttributeName("current-page")]
        public int CurrentPage { get; set; }

        [HtmlAttributeName("total-pages")]
        public int TotalPages { get; set; }

        // У тебя category — строка (NormalizedName)
        [HtmlAttributeName("category")]
        public string? Category { get; set; }

        // На случай использования пейджера на Razor Page (Admin/Index)
        [HtmlAttributeName("admin")]
        public bool Admin { get; set; }

        public override void Process(TagHelperContext context, TagHelperOutput output)
        {
            if (TotalPages <= 1)
            {
                output.SuppressOutput();
                return;
            }

            output.TagName = "nav";
            var ul = new TagBuilder("ul");
            ul.AddCssClass("pagination");
            ul.AddCssClass("justify-content-center");

            // Prev
            ul.InnerHtml.AppendHtml(BuildPageItem("«", Math.Max(1, CurrentPage - 1), disabled: CurrentPage == 1));

            // Pages
            for (int i = 1; i <= TotalPages; i++)
            {
                ul.InnerHtml.AppendHtml(BuildPageItem(i.ToString(), i, active: i == CurrentPage));
            }

            // Next
            ul.InnerHtml.AppendHtml(BuildPageItem("»", Math.Min(TotalPages, CurrentPage + 1), disabled: CurrentPage == TotalPages));

            output.Content.AppendHtml(ul);
        }

        private TagBuilder BuildPageItem(string text, int page, bool active = false, bool disabled = false)
        {
            var li = new TagBuilder("li");
            li.AddCssClass("page-item");
            if (active) li.AddCssClass("active");
            if (disabled) li.AddCssClass("disabled");

            var a = new TagBuilder("a");
            a.AddCssClass("page-link");
            a.Attributes["href"] = BuildHref(page);
            a.InnerHtml.Append(text);

            li.InnerHtml.AppendHtml(a);
            return li;
        }

        private string BuildHref(int page)
        {
            var httpContext = _httpContextAccessor.HttpContext!;

            if (Admin)
            {
                // Для Razor Page: /Admin/Index?pageNo=...&category=...
                return _linkGenerator.GetPathByPage(
                    httpContext,
                    page: "/Admin/Index",
                    values: new { pageNo = page, category = Category }
                );
            }

            // По умолчанию — на контроллер Car/Index
            return _linkGenerator.GetPathByAction(
                httpContext,
                action: "Index",
                controller: "Car",
                values: new { pageNo = page, category = Category }
            );
        }
    }
}
